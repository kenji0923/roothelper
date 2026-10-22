# Agent instructions

## Coding rules

### C++

- Use C++17.
- Follow the Google C++ Style Guide.
- When appropriate, follow the project structure defined by [cppprojbuilder](https://github.com/kenji0923/cppprojbuilder). Look for a local checkout or installation, starting with `~/.local/` and `~/opt/`. If it cannot be found, ask the user how to proceed.

#### CERN ROOT (with roothelper)

This repository is [roothelper](https://github.com/kenji0923/roothelper) itself. Code here that draws plots (tests, examples, scripts) uses its own functions, and changes to the library keep the rules below working for projects that use it.

- For projects that use CERN ROOT, use roothelper for data management and plot formatting.
- Use sentence case for axis titles, for example: `Frequency #minus 1,234,567 (GHz)`.
- Use a default line width of 1.
- Prefer roothelper's automatic formatting over manual adjustments to text sizes, margins, line widths, and marker sizes. Inspect the roothelper codebase and call the appropriate formatting functions.
- When a vector format other than PDF is required, use roothelper's conversion script (`script/ProcessPDF`, installed to `<prefix>/bin`) to generate SVG.
- Before embedding a PDF in software that does not handle PDF page rotation correctly, use the same script to normalize the rotation while preserving the intended appearance.
