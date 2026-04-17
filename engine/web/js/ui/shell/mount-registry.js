/**
 * Simple registry for shell mount points and named regions.
 */
export function createMountRegistry() {
    const regions = new Map();
    const mounts = new Map();

    return {
        registerRegion(name, element) {
            if (element) regions.set(name, element);
            return element;
        },
        registerMount(name, element) {
            if (element) mounts.set(name, element);
            return element;
        },
        getRegion(name) {
            return regions.get(name) || null;
        },
        getMount(name) {
            return mounts.get(name) || null;
        },
        listRegions() {
            return Array.from(regions.keys());
        },
        listMounts() {
            return Array.from(mounts.keys());
        },
    };
}
