export function isEditableTarget(target: EventTarget | null): boolean;
export function isShortcutBlockedTarget(target: EventTarget | null): boolean;
export function registerWorkspaceShortcuts(id: string, readRows: () => {action: string; code: string}[]): () => void;
export function getWorkspaceShortcuts(id?: string): {action: string; code: string}[];
export function setShortcutWorkspace(id: string): void;
export const DASHBOARD_SHORTCUTS: ReadonlyArray<{group: string; rows: ReadonlyArray<{keys: string[]; label: string}>}>;
