/** One reversible owner for a component's listeners and scheduled work. */
export class LifetimeScope {
    constructor() {
        this.disposed = false;
        this._releases = new Set();
    }

    /** Register a release; the returned function is itself idempotent. */
    defer(callback) {
        let active = true;
        const release = () => {
            if (!active) return;
            active = false;
            this._releases.delete(release);
            callback();
        };
        if (this.disposed) release();
        else this._releases.add(release);
        return release;
    }

    on(target, type, listener, options) {
        if (!target || this.disposed) return () => {};
        target.addEventListener(type, listener, options);
        return this.defer(() => target.removeEventListener(type, listener, options));
    }

    /** Schedule one frame; returns a cancel function, not a numeric frame ID. */
    frame(callback) {
        if (this.disposed) return () => {};
        const id = requestAnimationFrame(time => {
            release();
            if (!this.disposed) callback(time);
        });
        const release = this.defer(() => cancelAnimationFrame(id));
        return release;
    }

    /** Schedule one timeout; completed work does not accumulate in the owner. */
    timeout(callback, delay = 0) {
        if (this.disposed) return () => {};
        const id = setTimeout(() => {
            release();
            if (!this.disposed) callback();
        }, delay);
        const release = this.defer(() => clearTimeout(id));
        return release;
    }

    dispose() {
        if (this.disposed) return;
        this.disposed = true;
        const errors = [];
        for (const release of [...this._releases].reverse()) {
            try { release(); } catch (error) { errors.push(error); }
        }
        if (errors.length) throw new AggregateError(errors, 'Component cleanup failed');
    }
}
