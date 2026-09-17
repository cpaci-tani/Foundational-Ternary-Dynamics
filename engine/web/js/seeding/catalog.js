/** Constructor-generated finite defaults only. Edited drafts are described by
 * the local compiler, so browser forms never synthesize bounds or defaults. */
const CATALOG_URL = new URL('./generated/finite-seeds.json', import.meta.url);
let cached = null;
export async function loadFiniteCatalog() {
    if (!cached) cached = fetch(CATALOG_URL).then(response => {
        if (!response.ok) throw new Error('Could not load the finite seeding catalog');
        return response.json();
    }).catch(error => {cached=null;throw error;});
    return cached;
}
export function resetFiniteCatalogForTests(catalog) {cached = catalog ? Promise.resolve(catalog) : null;}
export function findFiniteSchema(catalog, scenarioId, size) {return catalog.schemas[`${scenarioId}@${size}`] || null;}
export function findFiniteBaseSchema(catalog, size) {
    const schema = catalog.schemas[`record-base@${size}`];
    if (!schema) throw new Error('No constructor-authored category base exists for this size.');
    return schema;
}
export function findComponentTemplates(catalog, size, baseSchema) {
    return new Map(Object.entries(catalog.componentTemplates[size]));
}
export function collectionRowBounds(meta, rows) {
    return {canAdd: rows < meta.maxRows, canRemove: rows > meta.minRows, defaultRow: meta.defaultRow};
}
