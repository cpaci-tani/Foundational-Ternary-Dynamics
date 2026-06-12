import js from "@eslint/js";
import tsParser from "@typescript-eslint/parser";
import tsPlugin from "@typescript-eslint/eslint-plugin";
import importPlugin from "eslint-plugin-import";

export default [
  {
    ignores: ["**/vendor/**/*.js", "**/node_modules/**/*"]
  },
  js.configs.recommended,
  {
    files: ["engine/web/js/**/*.js", "engine/web/js/**/*.ts"],
    plugins: {
      import: importPlugin,
    },
    languageOptions: {
      ecmaVersion: 2022,
      sourceType: "module",
      globals: {
        window: "readonly",
        document: "readonly",
        localStorage: "readonly",
        setInterval: "readonly",
        clearInterval: "readonly",
        setTimeout: "readonly",
        clearTimeout: "readonly",
        console: "readonly",
        performance: "readonly",
        HTMLElement: "readonly",
        HTMLInputElement: "readonly",
        HTMLSpanElement: "readonly",
        HTMLButtonElement: "readonly",
        HTMLSelectElement: "readonly",
        Element: "readonly",
        Float32Array: "readonly",
        Float64Array: "readonly",
        Uint8Array: "readonly",
        Int32Array: "readonly",
        Math: "readonly",
        isFinite: "readonly",
        isNaN: "readonly",
        Number: "readonly",
        requestAnimationFrame: "readonly",
        cancelAnimationFrame: "readonly",
        CustomEvent: "readonly",
        EventTarget: "readonly",
        Object: "readonly",
        Map: "readonly",
        Set: "readonly",
        Event: "readonly",
        URL: "readonly",
        fetch: "readonly",
        Blob: "readonly",
        ResizeObserver: "readonly",
        WebSocket: "readonly",
        JSON: "readonly",
        Array: "readonly",
        String: "readonly",
        Boolean: "readonly",
        Error: "readonly",
        alert: "readonly",
        Image: "readonly",
        setTimeout: "readonly",
        MutationObserver: "readonly",
        uPlot: "readonly"
      },
    },
    rules: {
      "import/extensions": ["error", "always", { js: "always", ts: "never" }],
      "no-unused-vars": ["warn", {
        args: "none",
        argsIgnorePattern: "^_",
        varsIgnorePattern: "^_",
        caughtErrorsIgnorePattern: "^_",
        destructuredArrayIgnorePattern: "^_"
      }],
      "no-console": ["warn", { allow: ["warn", "error", "info"] }],
      "no-empty": "off",
      "no-case-declarations": "off",
      "no-useless-escape": "off",
      "no-undef": "off",
      "no-loss-of-precision": "off"
    },
  },
  {
    files: ["engine/web/js/**/*.ts"],
    languageOptions: {
      parser: tsParser,
    },
    plugins: {
      "@typescript-eslint": tsPlugin,
    },
    rules: {
      ...tsPlugin.configs.recommended.rules,
    },
  },
];
