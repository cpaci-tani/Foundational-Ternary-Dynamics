/**
 * Periodic Table Data — all 118 elements.
 *
 * Each entry: { Z, symbol, name, neutrons, maxBonds, color, row, col }
 *   Z        — atomic number (1–118)
 *   symbol   — IUPAC symbol
 *   name     — element name
 *   neutrons — most stable isotope neutron count
 *   maxBonds — typical maximum covalent bonds (0 for noble gases)
 *   color    — CPK-convention [r,g,b] in 0–1 range
 *   row,col  — position in standard 18-column periodic table layout (1-indexed)
 *              Lanthanides: row 8, Actinides: row 9
 */

// prettier-ignore
const ELEMENTS = [
//  Z   sym    name                N   bonds  [r, g, b]                  row col
    {Z:1,   symbol:'H',  name:'Hydrogen',      neutrons:0,   maxBonds:1, color:[1.00, 1.00, 1.00], row:1, col:1},
    {Z:2,   symbol:'He', name:'Helium',         neutrons:2,   maxBonds:0, color:[0.85, 1.00, 1.00], row:1, col:18},
    {Z:3,   symbol:'Li', name:'Lithium',        neutrons:4,   maxBonds:1, color:[0.80, 0.50, 1.00], row:2, col:1},
    {Z:4,   symbol:'Be', name:'Beryllium',      neutrons:5,   maxBonds:2, color:[0.76, 1.00, 0.00], row:2, col:2},
    {Z:5,   symbol:'B',  name:'Boron',          neutrons:6,   maxBonds:3, color:[1.00, 0.71, 0.71], row:2, col:13},
    {Z:6,   symbol:'C',  name:'Carbon',         neutrons:6,   maxBonds:4, color:[0.56, 0.56, 0.56], row:2, col:14},
    {Z:7,   symbol:'N',  name:'Nitrogen',       neutrons:7,   maxBonds:3, color:[0.19, 0.31, 0.97], row:2, col:15},
    {Z:8,   symbol:'O',  name:'Oxygen',         neutrons:8,   maxBonds:2, color:[1.00, 0.05, 0.05], row:2, col:16},
    {Z:9,   symbol:'F',  name:'Fluorine',       neutrons:10,  maxBonds:1, color:[0.56, 0.88, 0.31], row:2, col:17},
    {Z:10,  symbol:'Ne', name:'Neon',           neutrons:10,  maxBonds:0, color:[0.70, 0.89, 0.96], row:2, col:18},
    {Z:11,  symbol:'Na', name:'Sodium',         neutrons:12,  maxBonds:1, color:[0.67, 0.36, 0.95], row:3, col:1},
    {Z:12,  symbol:'Mg', name:'Magnesium',      neutrons:12,  maxBonds:2, color:[0.54, 1.00, 0.00], row:3, col:2},
    {Z:13,  symbol:'Al', name:'Aluminium',      neutrons:14,  maxBonds:3, color:[0.75, 0.65, 0.65], row:3, col:13},
    {Z:14,  symbol:'Si', name:'Silicon',        neutrons:14,  maxBonds:4, color:[0.94, 0.78, 0.63], row:3, col:14},
    {Z:15,  symbol:'P',  name:'Phosphorus',     neutrons:16,  maxBonds:3, color:[1.00, 0.50, 0.00], row:3, col:15},
    {Z:16,  symbol:'S',  name:'Sulfur',         neutrons:16,  maxBonds:2, color:[1.00, 1.00, 0.19], row:3, col:16},
    {Z:17,  symbol:'Cl', name:'Chlorine',       neutrons:18,  maxBonds:1, color:[0.12, 0.94, 0.12], row:3, col:17},
    {Z:18,  symbol:'Ar', name:'Argon',          neutrons:22,  maxBonds:0, color:[0.50, 0.82, 0.89], row:3, col:18},
    // Period 4
    {Z:19,  symbol:'K',  name:'Potassium',      neutrons:20,  maxBonds:1, color:[0.56, 0.25, 0.83], row:4, col:1},
    {Z:20,  symbol:'Ca', name:'Calcium',        neutrons:20,  maxBonds:2, color:[0.24, 1.00, 0.00], row:4, col:2},
    {Z:21,  symbol:'Sc', name:'Scandium',       neutrons:24,  maxBonds:3, color:[0.90, 0.90, 0.90], row:4, col:3},
    {Z:22,  symbol:'Ti', name:'Titanium',       neutrons:26,  maxBonds:4, color:[0.75, 0.76, 0.78], row:4, col:4},
    {Z:23,  symbol:'V',  name:'Vanadium',       neutrons:28,  maxBonds:5, color:[0.65, 0.65, 0.67], row:4, col:5},
    {Z:24,  symbol:'Cr', name:'Chromium',       neutrons:28,  maxBonds:6, color:[0.54, 0.60, 0.78], row:4, col:6},
    {Z:25,  symbol:'Mn', name:'Manganese',      neutrons:30,  maxBonds:4, color:[0.61, 0.48, 0.78], row:4, col:7},
    {Z:26,  symbol:'Fe', name:'Iron',           neutrons:30,  maxBonds:3, color:[0.88, 0.40, 0.20], row:4, col:8},
    {Z:27,  symbol:'Co', name:'Cobalt',         neutrons:32,  maxBonds:3, color:[0.94, 0.56, 0.63], row:4, col:9},
    {Z:28,  symbol:'Ni', name:'Nickel',         neutrons:30,  maxBonds:3, color:[0.31, 0.82, 0.31], row:4, col:10},
    {Z:29,  symbol:'Cu', name:'Copper',         neutrons:34,  maxBonds:2, color:[0.78, 0.50, 0.20], row:4, col:11},
    {Z:30,  symbol:'Zn', name:'Zinc',           neutrons:34,  maxBonds:2, color:[0.49, 0.50, 0.69], row:4, col:12},
    {Z:31,  symbol:'Ga', name:'Gallium',        neutrons:38,  maxBonds:3, color:[0.76, 0.56, 0.56], row:4, col:13},
    {Z:32,  symbol:'Ge', name:'Germanium',      neutrons:42,  maxBonds:4, color:[0.40, 0.56, 0.56], row:4, col:14},
    {Z:33,  symbol:'As', name:'Arsenic',        neutrons:42,  maxBonds:3, color:[0.74, 0.50, 0.89], row:4, col:15},
    {Z:34,  symbol:'Se', name:'Selenium',       neutrons:46,  maxBonds:2, color:[1.00, 0.63, 0.00], row:4, col:16},
    {Z:35,  symbol:'Br', name:'Bromine',        neutrons:44,  maxBonds:1, color:[0.65, 0.16, 0.16], row:4, col:17},
    {Z:36,  symbol:'Kr', name:'Krypton',        neutrons:48,  maxBonds:0, color:[0.36, 0.72, 0.82], row:4, col:18},
    // Period 5
    {Z:37,  symbol:'Rb', name:'Rubidium',       neutrons:48,  maxBonds:1, color:[0.44, 0.18, 0.69], row:5, col:1},
    {Z:38,  symbol:'Sr', name:'Strontium',      neutrons:50,  maxBonds:2, color:[0.00, 1.00, 0.00], row:5, col:2},
    {Z:39,  symbol:'Y',  name:'Yttrium',        neutrons:50,  maxBonds:3, color:[0.58, 1.00, 1.00], row:5, col:3},
    {Z:40,  symbol:'Zr', name:'Zirconium',      neutrons:50,  maxBonds:4, color:[0.58, 0.88, 0.88], row:5, col:4},
    {Z:41,  symbol:'Nb', name:'Niobium',        neutrons:52,  maxBonds:5, color:[0.45, 0.76, 0.79], row:5, col:5},
    {Z:42,  symbol:'Mo', name:'Molybdenum',     neutrons:54,  maxBonds:6, color:[0.33, 0.71, 0.71], row:5, col:6},
    {Z:43,  symbol:'Tc', name:'Technetium',     neutrons:55,  maxBonds:4, color:[0.23, 0.62, 0.62], row:5, col:7},
    {Z:44,  symbol:'Ru', name:'Ruthenium',      neutrons:57,  maxBonds:4, color:[0.14, 0.56, 0.56], row:5, col:8},
    {Z:45,  symbol:'Rh', name:'Rhodium',        neutrons:58,  maxBonds:3, color:[0.04, 0.49, 0.55], row:5, col:9},
    {Z:46,  symbol:'Pd', name:'Palladium',      neutrons:60,  maxBonds:4, color:[0.00, 0.41, 0.52], row:5, col:10},
    {Z:47,  symbol:'Ag', name:'Silver',         neutrons:60,  maxBonds:1, color:[0.75, 0.75, 0.75], row:5, col:11},
    {Z:48,  symbol:'Cd', name:'Cadmium',        neutrons:64,  maxBonds:2, color:[1.00, 0.85, 0.56], row:5, col:12},
    {Z:49,  symbol:'In', name:'Indium',         neutrons:66,  maxBonds:3, color:[0.65, 0.46, 0.45], row:5, col:13},
    {Z:50,  symbol:'Sn', name:'Tin',            neutrons:70,  maxBonds:4, color:[0.40, 0.50, 0.50], row:5, col:14},
    {Z:51,  symbol:'Sb', name:'Antimony',       neutrons:70,  maxBonds:3, color:[0.62, 0.39, 0.71], row:5, col:15},
    {Z:52,  symbol:'Te', name:'Tellurium',      neutrons:78,  maxBonds:2, color:[0.83, 0.48, 0.00], row:5, col:16},
    {Z:53,  symbol:'I',  name:'Iodine',         neutrons:74,  maxBonds:1, color:[0.58, 0.00, 0.58], row:5, col:17},
    {Z:54,  symbol:'Xe', name:'Xenon',          neutrons:77,  maxBonds:0, color:[0.26, 0.62, 0.69], row:5, col:18},
    // Period 6
    {Z:55,  symbol:'Cs', name:'Caesium',        neutrons:78,  maxBonds:1, color:[0.34, 0.09, 0.56], row:6, col:1},
    {Z:56,  symbol:'Ba', name:'Barium',         neutrons:81,  maxBonds:2, color:[0.00, 0.79, 0.00], row:6, col:2},
    // Lanthanides (row 8 in layout)
    {Z:57,  symbol:'La', name:'Lanthanum',      neutrons:82,  maxBonds:3, color:[0.44, 0.83, 1.00], row:8, col:3},
    {Z:58,  symbol:'Ce', name:'Cerium',         neutrons:82,  maxBonds:4, color:[1.00, 1.00, 0.78], row:8, col:4},
    {Z:59,  symbol:'Pr', name:'Praseodymium',   neutrons:82,  maxBonds:4, color:[0.85, 1.00, 0.78], row:8, col:5},
    {Z:60,  symbol:'Nd', name:'Neodymium',      neutrons:84,  maxBonds:3, color:[0.78, 1.00, 0.78], row:8, col:6},
    {Z:61,  symbol:'Pm', name:'Promethium',     neutrons:84,  maxBonds:3, color:[0.64, 1.00, 0.78], row:8, col:7},
    {Z:62,  symbol:'Sm', name:'Samarium',       neutrons:88,  maxBonds:3, color:[0.56, 1.00, 0.78], row:8, col:8},
    {Z:63,  symbol:'Eu', name:'Europium',       neutrons:90,  maxBonds:3, color:[0.38, 1.00, 0.78], row:8, col:9},
    {Z:64,  symbol:'Gd', name:'Gadolinium',     neutrons:93,  maxBonds:3, color:[0.27, 1.00, 0.78], row:8, col:10},
    {Z:65,  symbol:'Tb', name:'Terbium',        neutrons:94,  maxBonds:3, color:[0.19, 1.00, 0.78], row:8, col:11},
    {Z:66,  symbol:'Dy', name:'Dysprosium',     neutrons:97,  maxBonds:3, color:[0.12, 1.00, 0.78], row:8, col:12},
    {Z:67,  symbol:'Ho', name:'Holmium',        neutrons:98,  maxBonds:3, color:[0.00, 1.00, 0.61], row:8, col:13},
    {Z:68,  symbol:'Er', name:'Erbium',         neutrons:99,  maxBonds:3, color:[0.00, 0.90, 0.46], row:8, col:14},
    {Z:69,  symbol:'Tm', name:'Thulium',        neutrons:100, maxBonds:3, color:[0.00, 0.83, 0.32], row:8, col:15},
    {Z:70,  symbol:'Yb', name:'Ytterbium',      neutrons:103, maxBonds:3, color:[0.00, 0.75, 0.22], row:8, col:16},
    {Z:71,  symbol:'Lu', name:'Lutetium',       neutrons:104, maxBonds:3, color:[0.00, 0.67, 0.14], row:6, col:3},
    // Back to period 6 main block
    {Z:72,  symbol:'Hf', name:'Hafnium',        neutrons:106, maxBonds:4, color:[0.30, 0.76, 1.00], row:6, col:4},
    {Z:73,  symbol:'Ta', name:'Tantalum',       neutrons:108, maxBonds:5, color:[0.30, 0.65, 1.00], row:6, col:5},
    {Z:74,  symbol:'W',  name:'Tungsten',       neutrons:110, maxBonds:6, color:[0.13, 0.58, 0.84], row:6, col:6},
    {Z:75,  symbol:'Re', name:'Rhenium',        neutrons:111, maxBonds:4, color:[0.15, 0.49, 0.67], row:6, col:7},
    {Z:76,  symbol:'Os', name:'Osmium',         neutrons:114, maxBonds:4, color:[0.15, 0.40, 0.59], row:6, col:8},
    {Z:77,  symbol:'Ir', name:'Iridium',        neutrons:115, maxBonds:4, color:[0.09, 0.33, 0.53], row:6, col:9},
    {Z:78,  symbol:'Pt', name:'Platinum',       neutrons:117, maxBonds:4, color:[0.82, 0.82, 0.88], row:6, col:10},
    {Z:79,  symbol:'Au', name:'Gold',           neutrons:118, maxBonds:3, color:[1.00, 0.82, 0.14], row:6, col:11},
    {Z:80,  symbol:'Hg', name:'Mercury',        neutrons:121, maxBonds:2, color:[0.72, 0.72, 0.82], row:6, col:12},
    {Z:81,  symbol:'Tl', name:'Thallium',       neutrons:123, maxBonds:3, color:[0.65, 0.33, 0.30], row:6, col:13},
    {Z:82,  symbol:'Pb', name:'Lead',           neutrons:125, maxBonds:4, color:[0.34, 0.35, 0.38], row:6, col:14},
    {Z:83,  symbol:'Bi', name:'Bismuth',        neutrons:126, maxBonds:3, color:[0.62, 0.31, 0.71], row:6, col:15},
    {Z:84,  symbol:'Po', name:'Polonium',       neutrons:125, maxBonds:2, color:[0.67, 0.36, 0.00], row:6, col:16},
    {Z:85,  symbol:'At', name:'Astatine',       neutrons:125, maxBonds:1, color:[0.46, 0.31, 0.27], row:6, col:17},
    {Z:86,  symbol:'Rn', name:'Radon',          neutrons:136, maxBonds:0, color:[0.26, 0.51, 0.59], row:6, col:18},
    // Period 7
    {Z:87,  symbol:'Fr', name:'Francium',       neutrons:136, maxBonds:1, color:[0.26, 0.00, 0.40], row:7, col:1},
    {Z:88,  symbol:'Ra', name:'Radium',         neutrons:138, maxBonds:2, color:[0.00, 0.49, 0.00], row:7, col:2},
    // Actinides (row 9 in layout)
    {Z:89,  symbol:'Ac', name:'Actinium',       neutrons:138, maxBonds:3, color:[0.44, 0.67, 0.98], row:9, col:3},
    {Z:90,  symbol:'Th', name:'Thorium',        neutrons:142, maxBonds:4, color:[0.00, 0.73, 1.00], row:9, col:4},
    {Z:91,  symbol:'Pa', name:'Protactinium',   neutrons:140, maxBonds:5, color:[0.00, 0.63, 1.00], row:9, col:5},
    {Z:92,  symbol:'U',  name:'Uranium',        neutrons:146, maxBonds:6, color:[0.00, 0.56, 1.00], row:9, col:6},
    {Z:93,  symbol:'Np', name:'Neptunium',      neutrons:144, maxBonds:5, color:[0.00, 0.50, 1.00], row:9, col:7},
    {Z:94,  symbol:'Pu', name:'Plutonium',      neutrons:150, maxBonds:4, color:[0.00, 0.42, 1.00], row:9, col:8},
    {Z:95,  symbol:'Am', name:'Americium',      neutrons:148, maxBonds:3, color:[0.33, 0.36, 0.95], row:9, col:9},
    {Z:96,  symbol:'Cm', name:'Curium',         neutrons:151, maxBonds:3, color:[0.47, 0.36, 0.89], row:9, col:10},
    {Z:97,  symbol:'Bk', name:'Berkelium',      neutrons:150, maxBonds:3, color:[0.54, 0.31, 0.89], row:9, col:11},
    {Z:98,  symbol:'Cf', name:'Californium',    neutrons:153, maxBonds:3, color:[0.63, 0.21, 0.83], row:9, col:12},
    {Z:99,  symbol:'Es', name:'Einsteinium',    neutrons:153, maxBonds:3, color:[0.70, 0.12, 0.83], row:9, col:13},
    {Z:100, symbol:'Fm', name:'Fermium',        neutrons:157, maxBonds:3, color:[0.70, 0.12, 0.73], row:9, col:14},
    {Z:101, symbol:'Md', name:'Mendelevium',    neutrons:157, maxBonds:3, color:[0.70, 0.05, 0.65], row:9, col:15},
    {Z:102, symbol:'No', name:'Nobelium',       neutrons:157, maxBonds:2, color:[0.74, 0.05, 0.53], row:9, col:16},
    {Z:103, symbol:'Lr', name:'Lawrencium',     neutrons:159, maxBonds:3, color:[0.78, 0.00, 0.40], row:7, col:3},
    // Back to period 7 main block
    {Z:104, symbol:'Rf', name:'Rutherfordium',  neutrons:157, maxBonds:4, color:[0.80, 0.00, 0.35], row:7, col:4},
    {Z:105, symbol:'Db', name:'Dubnium',        neutrons:157, maxBonds:5, color:[0.82, 0.00, 0.31], row:7, col:5},
    {Z:106, symbol:'Sg', name:'Seaborgium',     neutrons:157, maxBonds:6, color:[0.85, 0.00, 0.27], row:7, col:6},
    {Z:107, symbol:'Bh', name:'Bohrium',        neutrons:155, maxBonds:4, color:[0.88, 0.00, 0.22], row:7, col:7},
    {Z:108, symbol:'Hs', name:'Hassium',        neutrons:157, maxBonds:4, color:[0.90, 0.00, 0.18], row:7, col:8},
    {Z:109, symbol:'Mt', name:'Meitnerium',     neutrons:157, maxBonds:3, color:[0.92, 0.00, 0.15], row:7, col:9},
    {Z:110, symbol:'Ds', name:'Darmstadtium',   neutrons:171, maxBonds:3, color:[0.93, 0.00, 0.12], row:7, col:10},
    {Z:111, symbol:'Rg', name:'Roentgenium',    neutrons:170, maxBonds:3, color:[0.94, 0.00, 0.09], row:7, col:11},
    {Z:112, symbol:'Cn', name:'Copernicium',    neutrons:173, maxBonds:2, color:[0.95, 0.00, 0.06], row:7, col:12},
    {Z:113, symbol:'Nh', name:'Nihonium',       neutrons:173, maxBonds:3, color:[0.96, 0.00, 0.04], row:7, col:13},
    {Z:114, symbol:'Fl', name:'Flerovium',      neutrons:175, maxBonds:4, color:[0.97, 0.00, 0.02], row:7, col:14},
    {Z:115, symbol:'Mc', name:'Moscovium',      neutrons:174, maxBonds:3, color:[0.98, 0.00, 0.01], row:7, col:15},
    {Z:116, symbol:'Lv', name:'Livermorium',    neutrons:177, maxBonds:2, color:[0.99, 0.00, 0.00], row:7, col:16},
    {Z:117, symbol:'Ts', name:'Tennessine',     neutrons:177, maxBonds:1, color:[0.99, 0.08, 0.08], row:7, col:17},
    {Z:118, symbol:'Og', name:'Oganesson',      neutrons:176, maxBonds:0, color:[0.99, 0.16, 0.16], row:7, col:18},
];

// Build lookup maps for fast access by Z
const _byZ = new Map();
for (const el of ELEMENTS) _byZ.set(el.Z, el);

/** Get element data by atomic number (Z). Returns undefined if Z is out of range. */
export function getElement(Z) { return _byZ.get(Z); }

/** Get element symbol by Z. Returns '?' if unknown. */
export function elementSymbol(Z) { const el = _byZ.get(Z); return el ? el.symbol : '?'; }

/** Get element name by Z. */
export function elementName(Z) { const el = _byZ.get(Z); return el ? el.name : `Element ${Z}`; }

/** Get CPK color [r,g,b] by Z. Returns gray for unknown. */
export function cpkColor(Z) { const el = _byZ.get(Z); return el ? el.color : [0.5, 0.5, 0.5]; }

/** Get default neutron count by Z. Falls back to Z if unknown. */
export function defaultNeutronCount(Z) { const el = _byZ.get(Z); return el ? el.neutrons : Z; }

/** Get typical max covalent bonds by Z. Falls back to 4 for unknown. */
export function maxBonds(Z) { const el = _byZ.get(Z); return el != null ? el.maxBonds : 4; }

/** Get periodic table position {row, col} by Z (1-indexed). */
export function tablePosition(Z) { const el = _byZ.get(Z); return el ? {row:el.row, col:el.col} : null; }

/** Return the full ELEMENTS array. */
export function allElements() { return ELEMENTS; }

/** Total number of elements. */
export const ELEMENT_COUNT = ELEMENTS.length;
