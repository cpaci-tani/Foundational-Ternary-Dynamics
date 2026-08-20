# Native-desktop third-party font: Inter

Source: https://github.com/rsms/inter (SIL Open Font License 1.1)
Version: 4.1 (tag v4.1)
File: extras/ttf/Inter-Regular.ttf from the Inter-4.1 release zip
Licence: OFL.txt (full OFL 1.1 text)

Why: SPEC_UI_V2 §9.2 — embed Inter so L1 draw-data vertex counts are
machine-independent. ProggyClean is not used for the v2 chrome.

Generated artifact: `font_inter_regular.inl`, produced by Dear ImGui's
`misc/fonts/binary_to_compressed_c.cpp`. Loaded via
`ImFontAtlas::AddFontFromMemoryCompressedTTF`.

Regenerate (from a vcvars 14.44 shell, repo root):

```
cl /nologo /O2 /Fe%TEMP%\binary_to_compressed_c.exe engine\thirdparty\imgui\misc\fonts\binary_to_compressed_c.cpp
%TEMP%\binary_to_compressed_c.exe engine\native_desktop\assets\Inter-Regular.ttf font_inter_regular > engine\native_desktop\assets\font_inter_regular.inl
```

Then rewrite the two generated comment lines so they use repo-relative
paths (no machine-local absolute path). Rebuild MANIFEST.sha256.

Hashes: MANIFEST.sha256 (enforced by FtdSourceLint.cmake).
