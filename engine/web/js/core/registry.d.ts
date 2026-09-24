interface ServiceRegistry {
    register(token: string, instance: unknown): void;
    get(token: string): any;
    unregister(token: string): void;
    clear(): void;
}
export const appRegistry: ServiceRegistry;
