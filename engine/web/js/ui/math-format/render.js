/**
 * renderMathInHtml — scan an already-escaped HTML string for LaTeX
 * delimiters (\\( ... \\) inline, \\[ ... \\] display), render each with
 * KaTeX, and substitute the rendered HTML back in place. Everything
 * outside delimiters is returned unchanged.
 *
 * The caller is responsible for escapeHtml-ing the text first — this
 * helper only touches the math spans. LaTeX source is trusted (content
 * is authored in data files, not user-supplied at runtime).
 *
 * If KaTeX is not loaded (CDN unreachable), the LaTeX source is left
 * untouched so the reader at least sees the raw math — not a regression
 * of the pre-integration state.
 */

const INLINE_RE = /\\\(([\s\S]+?)\\\)/g;
const DISPLAY_RE = /\\\[([\s\S]+?)\\\]/g;

function getKatex() {
    if (typeof window === 'undefined') return null;
    return window.katex || null;
}

function renderOne(latex, displayMode) {
    const katex = getKatex();
    if (!katex) return null;
    try {
        return katex.renderToString(latex, {
            throwOnError: false,
            displayMode,
            output: 'html',
        });
    } catch (_err) {
        return null;
    }
}

export function renderMathInHtml(escapedHtml) {
    if (typeof escapedHtml !== 'string' || escapedHtml.length === 0) return escapedHtml;

    // Display math first so an inline regex doesn't partially eat a \\[ ... \\] block.
    let out = escapedHtml.replace(DISPLAY_RE, (match, latex) => {
        const rendered = renderOne(latex, true);
        return rendered != null ? rendered : match;
    });
    out = out.replace(INLINE_RE, (match, latex) => {
        const rendered = renderOne(latex, false);
        return rendered != null ? rendered : match;
    });
    return out;
}
