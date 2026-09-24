/** Public lifetime contract for strictly checked presentation modules. */
export class LifetimeScope {
    disposed: boolean;
    defer(callback: () => void): () => void;
    on(target: EventTarget | null | undefined, type: string, listener: (event: any) => void, options?: boolean | AddEventListenerOptions): () => void;
    frame(callback: (time: number) => void): () => void;
    timeout(callback: () => void, delay?: number): () => void;
    dispose(): void;
}
