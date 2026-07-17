import './script-studio.css';
import * as monaco from 'monaco-editor';
import { Terminal } from '@xterm/xterm';
import { FitAddon } from '@xterm/addon-fit';
import { ClipboardAddon } from '@xterm/addon-clipboard';

globalThis.MonacoEnvironment = {
  getWorkerUrl(_moduleId, label) {
    return label === 'typescript' || label === 'javascript'
      ? './typescript.worker.js'
      : './editor.worker.js';
  }
};

monaco.languages.register({ id: 'powershell' });
monaco.languages.setMonarchTokensProvider('powershell', {
  ignoreCase: true,
  tokenizer: {
    root: [
      [/#.*$/, 'comment'],
      [/\$[a-z_][\w:]*/i, 'variable'],
      [/'[^']*'/, 'string'],
      [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],
      [/\b(function|filter|param|begin|process|end|if|elseif|else|switch|foreach|for|while|do|until|try|catch|finally|throw|return|break|continue|class|enum|using)\b/, 'keyword'],
      [/[a-z]+-[a-z][\w-]*/i, 'type.identifier'],
      [/\d+(?:\.\d+)?/, 'number'],
      [/[{}()[\]]/, '@brackets']
    ],
    string: [
      [/[^"`$]+/, 'string'],
      [/`./, 'string.escape'],
      [/\$[a-z_][\w:]*/i, 'variable'],
      [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
    ]
  }
});

monaco.languages.register({ id: 'bat' });
monaco.languages.setMonarchTokensProvider('bat', {
  ignoreCase: true,
  tokenizer: {
    root: [
      [/^\s*(rem\b.*|::.*)$/i, 'comment'],
      [/%[^%]+%|![^!]+!/, 'variable'],
      [/\b(if|else|for|in|do|goto|call|set|setlocal|endlocal|exit|echo|shift)\b/i, 'keyword'],
      [/"[^"\r\n]*"/, 'string'],
      [/\b\d+\b/, 'number']
    ]
  }
});

const languageIds = {
  PowerShell: 'powershell',
  Python: 'python',
  Bash: 'shell',
  CommandPrompt: 'bat',
  JavaScript: 'javascript',
  TypeScript: 'typescript'
};

const samples = {
  PowerShell: "Get-ComputerInfo | Select-Object WindowsProductName, WindowsVersion, OsArchitecture\n",
  Python: "import platform\nimport sys\n\nprint(sys.version)\nprint(platform.platform())\n",
  Bash: "uname -a\nprintf 'Shell: %s\\n' \"$SHELL\"\n",
  CommandPrompt: "ver\nwhoami\n",
  JavaScript: "console.log(process.version);\nconsole.log(process.platform);\n",
  TypeScript: "const greeting: string = 'Sift Script Studio';\nconsole.log(greeting);\n"
};

let currentLanguage = 'PowerShell';
const model = monaco.editor.createModel(samples[currentLanguage], languageIds[currentLanguage], monaco.Uri.parse('inmemory://sift/scratch.ps1'));
const editor = monaco.editor.create(document.getElementById('editor'), {
  model,
  theme: 'vs-dark',
  automaticLayout: true,
  fontFamily: 'Cascadia Mono, Consolas, monospace',
  fontSize: 13,
  lineHeight: 21,
  minimap: { enabled: true },
  suggest: { showWords: true },
  quickSuggestions: { other: true, comments: false, strings: true },
  tabCompletion: 'on',
  formatOnPaste: true,
  formatOnType: true,
  bracketPairColorization: { enabled: true },
  guides: { bracketPairs: true, indentation: true },
  padding: { top: 12, bottom: 12 },
  scrollBeyondLastLine: false,
  accessibilitySupport: 'auto'
});

const terminal = new Terminal({
  convertEol: true,
  cursorBlink: false,
  disableStdin: true,
  fontFamily: 'Cascadia Mono, Consolas, monospace',
  fontSize: 12,
  lineHeight: 1.25,
  scrollback: 3000,
  screenReaderMode: true,
  theme: {
    background: '#0d100e',
    foreground: '#b7c4b1',
    cursor: '#d4a65a',
    selectionBackground: '#526054aa',
    red: '#dc7e69',
    yellow: '#d4a65a',
    green: '#7b9a74'
  }
});
const fitAddon = new FitAddon();
terminal.loadAddon(fitAddon);
terminal.loadAddon(new ClipboardAddon());
terminal.open(document.getElementById('terminal'));
fitAddon.fit();
terminal.writeln('\x1b[32mSift Script Studio ready.\x1b[0m');
terminal.writeln('Documents are analyzed in memory. Authored-script execution is not enabled in this safety phase.');

const resizeObserver = new ResizeObserver(() => {
  try { fitAddon.fit(); } catch { /* The terminal can be between layouts. */ }
});
resizeObserver.observe(document.getElementById('terminal-region'));

function setLanguage(language, preserveText) {
  if (!languageIds[language]) return;
  const text = preserveText ? model.getValue() : (samples[language] ?? '');
  currentLanguage = language;
  monaco.editor.setModelLanguage(model, languageIds[language]);
  if (!preserveText) model.setValue(text);
  monaco.editor.setModelMarkers(model, 'sift', []);
  renderProblems([], false);
}

function severity(value) {
  if (value === 'Blocked' || value === 'Error') return monaco.MarkerSeverity.Error;
  if (value === 'Warning') return monaco.MarkerSeverity.Warning;
  return monaco.MarkerSeverity.Info;
}

function setDiagnostics(diagnostics) {
  const values = Array.isArray(diagnostics) ? diagnostics : [];
  monaco.editor.setModelMarkers(model, 'sift', values.map(item => ({
    startLineNumber: Math.max(1, item.line),
    startColumn: Math.max(1, item.column),
    endLineNumber: Math.max(1, item.line),
    endColumn: Math.max(2, item.column + 1),
    severity: severity(item.severity),
    source: item.source,
    code: item.code,
    message: item.message
  })));
  renderProblems(values, true);
}

function renderProblems(diagnostics, analyzed) {
  const host = document.getElementById('problems');
  const empty = document.getElementById('problems-empty');
  host.replaceChildren();
  empty.style.display = diagnostics.length === 0 ? 'block' : 'none';
  empty.textContent = analyzed
    ? 'No syntax or Sift policy findings were reported.'
    : 'Analyze the document to populate syntax and Sift policy diagnostics.';
  document.getElementById('problem-count').textContent = `${diagnostics.length} finding${diagnostics.length === 1 ? '' : 's'}`;
  for (const item of diagnostics) {
    const row = document.createElement('div');
    row.className = `problem ${String(item.severity).toLowerCase()}`;
    row.tabIndex = 0;
    row.setAttribute('role', 'listitem');
    const message = document.createElement('div');
    message.className = 'problem-line';
    message.textContent = item.message;
    const meta = document.createElement('div');
    meta.className = 'problem-meta';
    meta.textContent = `Ln ${item.line}, Col ${item.column} · ${item.source} · ${item.code}`;
    row.append(message, meta);
    const navigate = () => {
      editor.setPosition({ lineNumber: Math.max(1, item.line), column: Math.max(1, item.column) });
      editor.revealPositionInCenter({ lineNumber: Math.max(1, item.line), column: Math.max(1, item.column) });
      editor.focus();
    };
    row.addEventListener('click', navigate);
    row.addEventListener('keydown', event => { if (event.key === 'Enter' || event.key === ' ') navigate(); });
    host.append(row);
  }
}

function writeTerminal(text, error = false) {
  const prefix = error ? '\x1b[31m' : '';
  const suffix = error ? '\x1b[0m' : '';
  const safe = String(text)
    .replaceAll('\r\n', '\n')
    .replace(/[\x00-\x08\x0b-\x1f\x7f-\x9f]/g, value => `\\x${value.charCodeAt(0).toString(16).padStart(2, '0')}`);
  for (const line of safe.split('\n')) terminal.writeln(`${prefix}${line}${suffix}`);
}

function post(message) {
  globalThis.chrome?.webview?.postMessage(message);
}

globalThis.chrome?.webview?.addEventListener('message', event => {
  const message = event.data;
  if (!message || typeof message.type !== 'string') return;
  switch (message.type) {
    case 'language.set':
      setLanguage(message.language, Boolean(message.preserveText));
      break;
    case 'diagnostics.set':
      setDiagnostics(message.diagnostics);
      break;
    case 'terminal.write':
      writeTerminal(message.text, Boolean(message.error));
      break;
    case 'terminal.clear':
      terminal.clear();
      break;
    case 'document.request':
      post({ type: 'document.response', requestId: message.requestId, text: model.getValue(), language: currentLanguage });
      break;
    case 'editor.focus':
      editor.focus();
      break;
  }
});

const menu = document.getElementById('terminal-menu');
function closeMenu() { menu.classList.remove('visible'); }
document.getElementById('terminal').addEventListener('contextmenu', event => {
  event.preventDefault();
  menu.style.left = `${Math.min(event.clientX, innerWidth - 250)}px`;
  menu.style.top = `${Math.min(event.clientY, innerHeight - 170)}px`;
  menu.classList.add('visible');
});
document.addEventListener('pointerdown', event => { if (!menu.contains(event.target)) closeMenu(); });
menu.addEventListener('click', async event => {
  const action = event.target?.dataset?.action;
  closeMenu();
  if (action === 'copy') {
    const selection = terminal.getSelection();
    if (selection) post({ type: 'clipboard.copy', text: selection });
  } else if (action === 'select-all') {
    terminal.selectAll();
  } else if (action === 'clear') {
    terminal.clear();
  } else if (action === 'open-working-directory') {
    post({ type: 'explorer.open-working-directory' });
  }
});

editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => post({ type: 'analysis.request' }));
post({ type: 'ready' });
