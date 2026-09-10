/** Read-only integer observations of one validated native hydro snapshot.
 * No evolution, constitutive model, pressure closure, or physical-unit conversion.
 */
const LAW = 'phi-hydro-staged-candidate-1';
const TABLE = 'abf25cf26072c03b5b7865fe84d3f31c270263d2c061e7bf3d81e27d783b5375';
const ENCODING = '3c10c134dadf3aa6f32f31ba588996e3c4755af67d4c804db567f5c4b361270c';
const UNSIGNED = /^(0|[1-9][0-9]*)$/;
const MAX_TICK = (1n << 64n) - 1n;
export const HYDRO_VELOCITIES = Object.freeze([[1,0,0],[1,0,0],[-1,0,0],[-1,0,0],[0,1,0],[0,1,0],
    [0,-1,0],[0,-1,0],[0,0,1],[0,0,1],[0,0,-1],[0,0,-1],[1,1,0],
    [1,-1,0],[-1,1,0],[-1,-1,0],[1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],
    [0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]].map(row => Object.freeze(row)));
const VELOCITIES = HYDRO_VELOCITIES;
const SECOND = VELOCITIES.map(([x,y,z]) => [x*x,y*y,z*z,x*y,x*z,y*z]);
const SIZES = { s:1, bank:192, sc:6, fcc:12, admitted_sc:3, admitted_fcc:6, gate_sc:3, gate_fcc:6 };
const demand = (condition, message) => { if (!condition) throw new TypeError(message); };
const integer = (value, lo, hi, name) => {
    demand(Number.isSafeInteger(value) && value >= lo && value <= hi, `invalid ${name}`);
    return value;
};
function ordinal(value, name) {
    demand(typeof value === 'string' && UNSIGNED.test(value) && value.length <= 20, `invalid ${name}`);
    const result = BigInt(value);
    demand(result <= MAX_TICK, `${name} exceeds uint64`);
    return result;
}
function keys(object, expected, name) {
    demand(object && typeof object === 'object' && !Array.isArray(object)
        && JSON.stringify(Object.keys(object).sort()) === JSON.stringify([...expected].sort()), `invalid ${name} fields`);
}
function encodedArray(text, bytes, name) {
    const padding = (3 - bytes % 3) % 3;
    demand(typeof text === 'string' && text.length === 4*Math.ceil(bytes/3)
        && !/[^A-Za-z0-9+/=]/.test(text), `invalid ${name} base64`);
    const body = text.length-padding;
    demand(text.indexOf('=') === (padding ? body : -1)
        && (!padding || text.endsWith('='.repeat(padding))), `invalid ${name} padding`);
    if (padding) {
        const last = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'.indexOf(text[body-1]);
        demand((last & (padding === 2 ? 15 : 3)) === 0, `noncanonical ${name} pad bits`);
    }
}
function decode(text) {
    const binary = atob(text);
    const raw = new Uint8Array(binary.length);
    for (let i = 0; i < binary.length; ++i) raw[i] = binary.charCodeAt(i);
    return raw;
}

/** Decode once per native generation. Returned reader privately owns observation
 * arrays; callers cannot mutate them or supply an unvalidated fast-path object.
 * Only bank/sc/fcc are decoded. Other native checkpoint arrays have framing checked.
 */
export function createFluidSnapshot(checkpoint) {
    demand(typeof checkpoint === 'string' && checkpoint.length <= 11_000_000, 'invalid fluid checkpoint size/type');
    const value = JSON.parse(checkpoint);
    keys(value, ['schema','law','table','encoding','boundary','L','microtick','arrays'], 'checkpoint');
    demand(value.schema === 'ftd-hydro-checkpoint-2' && value.law === LAW && value.table === TABLE
        && value.encoding === ENCODING && value.boundary === 'periodic', 'foreign fluid checkpoint');
    const L = integer(value.L, 3, 32, 'fluid lattice size');
    const tick = ordinal(value.microtick, 'microtick');
    const sites = L**3;
    keys(value.arrays, Object.keys(SIZES), 'checkpoint array');
    for (const [name, size] of Object.entries(SIZES)) encodedArray(value.arrays[name], sites*size, name);
    const bank = decode(value.arrays.bank), sc = decode(value.arrays.sc), fcc = decode(value.arrays.fcc);
    // Collapse passive labels once. Exclusion bounds every output sum by 48*32^3;
    // even total field+relation work is <=66*32^3<2^22, so Number sums are exact.
    const occupied = new Uint8Array(sites*48);
    for (let site = 0; site < sites; ++site) for (let pol = 0; pol < 2; ++pol) {
        for (let v = 0; v < 24; ++v) {
            let count = 0;
            for (let k = 0; k < 4; ++k) {
                const bit = bank[site*192+pol*96+k*24+v];
                demand(bit <= 1, 'noncanonical bank Boolean');
                count += bit;
            }
            demand(count <= 1, 'fluid velocity exclusion violated');
            occupied[site*48+pol*24+v] = count;
        }
    }
    for (const raw of [sc,fcc]) for (const symbol of raw) demand(symbol <= 8, 'invalid fluid relation alphabet');
    const microtick = value.microtick;
    return Object.freeze({
        L: String(L), microtick,
        observe(width, source = {status:'unavailable',reason:'No completed advance in this owner.'}) {
            const w = ordinal(width, 'width');
            demand(w > 0n && w <= BigInt(L) && BigInt(L)%w === 0n, 'width must divide fluid lattice size');
            if (source.status === 'invalid') throw new TypeError(source.reason);
            demand(source.status === 'unavailable' || (source.status === 'available'
                && source.to_microtick === microtick), 'fluid source interval is stale');
            const blockWidth = Number(w), side = L/blockWidth;
            const blocks = Array.from({length:side**3}, () => ({
                field_tokens:[0,0], momentum:[[0,0,0],[0,0,0]],
                second_moment:[Array(6).fill(0),Array(6).fill(0)], relation_tokens:0,
            }));
            const totalN = [0,0], totalP = [[0,0,0],[0,0,0]];
            let scCount = 0, fccCount = 0;
            for (let site = 0; site < sites; ++site) {
                const x = Math.floor(site/(L*L)), y = Math.floor(site/L)%L, z = site%L;
                const b = (Math.floor(x/blockWidth)*side+Math.floor(y/blockWidth))*side+Math.floor(z/blockWidth);
                const block = blocks[b];
                for (let pol = 0; pol < 2; ++pol) for (let v = 0; v < 24; ++v) {
                    if (!occupied[site*48+pol*24+v]) continue;
                    ++block.field_tokens[pol]; ++totalN[pol];
                    for (let a = 0; a < 3; ++a) {block.momentum[pol][a] += VELOCITIES[v][a]; totalP[pol][a] += VELOCITIES[v][a];}
                    for (let a = 0; a < 6; ++a) block.second_moment[pol][a] += SECOND[v][a];
                }
                // Relation slots are assigned to their stored owner site's block.
                for (let c = 0; c < 6; ++c) if (sc[site*6+c] !== 4) {++block.relation_tokens; ++scCount;}
                for (let c = 0; c < 12; ++c) if (fcc[site*12+c] !== 4) {++block.relation_tokens; ++fccCount;}
            }
            return {
                schema:'ftd-hydro-fluid-snapshot-v1', status:'exact_observation', observation_phase:'snapshot',
                law:LAW, table_hash:TABLE, encoding_hash:ENCODING, L:String(L), width,
                side:String(side), block_sites:String(blockWidth**3), microtick, phase:String(tick%4n),
                second_moment_order:['xx','yy','zz','xy','xz','yz'],
                blocks:blocks.map(block => ({field_tokens:block.field_tokens.map(String),
                    momentum:block.momentum.map(row => row.map(String)),
                    second_moment:block.second_moment.map(row => row.map(String)), relation_tokens:String(block.relation_tokens)})),
                ledger:{field_tokens_by_polarity:totalN.map(String),momentum_by_polarity:totalP.map(row => row.map(String)),
                    relation_tokens_sc:String(scCount),relation_tokens_fcc:String(fccCount),
                    work_units:String(totalN[0]+totalN[1]+scCount+fccCount)},
                source:structuredClone(source),
            };
        },
    });
}

/** Exact field-source change from the actual successful native advance's events.
 * Returned signs are field changes; transferred_tokens is the positive relation gain.
 */
export function summarizeFluidAdvance(result, microticks) {
    const ticks = ordinal(microticks, 'advance microticks');
    demand(ticks > 0n && ticks <= 64n, 'unsupported fluid source interval');
    const d = result?.diagnostics;
    demand(d?.law === LAW && d.table_hash === TABLE, 'foreign source diagnostics');
    const L = Number(ordinal(d.L, 'source L'));
    integer(L, 3, 32, 'source lattice size');
    const to = ordinal(d.microtick, 'source microtick');
    demand(to >= ticks && d.phase === String(to%4n), 'source clock mismatch');
    keys(result.events, ['absorptions','collisions','crossings','gate_holds'], 'native events');
    for (const rows of Object.values(result.events)) demand(Array.isArray(rows), 'invalid native event array');
    const absorbed = result.events.absorptions;
    demand(absorbed.length <= 48*L**3, 'source count exceeds field capacity');
    const counts = [0,0], momentum = [[0,0,0],[0,0,0]];
    for (const row of absorbed) {
        demand(Array.isArray(row) && row.length === 6, 'invalid native absorption');
        const [x,c,kind,owner,a,b] = row;
        integer(x,0,L**3-1,'absorption site'); integer(c,0,191,'absorption channel');
        integer(kind,0,1,'absorption kind'); integer(owner,0,L**3-1,'relation owner');
        integer(a,0,2,'relation axis'); integer(b,0,kind === 0 ? 0 : 1,'relation diagonal');
        demand(Math.floor((c%96)/24) === 2, 'absorption requires passive phase two');
        const pol = Math.floor(c/96), v = c%24;
        --counts[pol];
        for (let axis = 0; axis < 3; ++axis) momentum[pol][axis] -= VELOCITIES[v][axis];
    }
    return {status:'available',from_microtick:String(to-ticks),to_microtick:String(to),microticks,
        transferred_tokens:String(absorbed.length),tokens_by_polarity:counts.map(String),
        momentum_by_polarity:momentum.map(row => row.map(String))};
}
