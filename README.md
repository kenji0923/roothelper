# roothelper

`roothelper` is a C++ library designed to simplify, automate, and beautify CERN ROOT graphics layouts and data serialization. It features aspect-ratio-corrected layout solvers, dynamic axis margins, automated ROOT file directory management, and helper wrappers for collection plotting.

---

## Key Capabilities

1. **Adaptive Graphics Layouts (`Graphics`)**
   * **Constant-Gap Y-Axis Title Position**: Solves the label width dynamically to compute axis title offsets, ensuring vertical titles never overlap tick labels or clip off-page.
   * **Dynamic Z-Axis Margin**: Automatically adjusts the right pad margin for 2D histograms to fit the color palette, tick marks, and scientific notation exponents safely.
   * **Predefined Presets**: Standardized aspect ratio layouts for paper publication sizes (e.g., `8pt` and `10pt` font sizing templates).

2. **Recursive Data Serialization (`DataSaver`)**
   * Automatically prints canvas layouts to `PDF` and `PNG` files.
   * Traverses canvas components recursively and serializes them to a single `.root` file under canvas-specific subdirectories (e.g., `data_c_xxx/`, `data_mg_xxx/`, `data_hs_xxx/`) to prevent root-directory clutter.

3. **Collection Wrappers & Utilities (`Container` & `Analysis`)**
   * **Polymorphic Wrappers**: Instantiates dynamic wrappers (`TMultiGraph`, `THStack`) that automatically cycle through distinct color palettes when children are appended.
   * **Arithmetic Combinations**: Handles error propagation and scaling for histograms and graphs point-by-point.

---

## API References & Manuals

Comprehensive references are provided in two formats:

### 1. Developer Handbook ([API_REFERENCE.md](file:///home/kshu/work/development/ROOT_helper/src/document/API_REFERENCE.md))
A clean, guide-oriented Markdown manual describing the core architecture, submodule functions, classes, parameter definitions, directory hierarchies, and layout formulas.

### 2. Doxygen C++ API Manual (Interactive HTML)
Fully detailed, interactive class and namespace documentation generated directly from inline comments in the public headers.

* **GitHub Pages (Automatic Deployment)**:
  Pushing to the `main` branch triggers the GitHub Actions workflow to build and host the Doxygen documentation automatically at your repository's GitHub Pages domain.
  
* **Local Compilation**:
  To compile and view the interactive documentation locally without requiring CERN ROOT to be installed:
  ```bash
  # Configure with DOCS_ONLY option
  cmake -B build -S . -DDOCS_ONLY=ON
  
  # Build the doc target
  cmake --build build --target doc
  
  # Open in browser
  open build/src/document/html/index.html
  ```

---

## Utility Scripts (`script/`)

To support development, testing, and system integration, `roothelper` includes the following scripts under the [script/](file:///home/kshu/work/development/ROOT_helper/script) directory:

* **`rooti`**: A Python command-line wrapper to execute ROOT macro scripts with dynamic argument type inference. It parses list structures, booleans, numbers, and strings from shell inputs, formatting a C++ invocation call like `script_path(arg1, arg2, ...)` and executing ROOT in batch mode (via `root -b -q`).
* **`CreateRootClangd`**: Automatically creates a local `.clangd` file containing ROOT compilation flags derived from `root-config --cflags`. This resolves ROOT symbols and headers correctly for clangd-based Language Server Protocol (LSP) configurations.
* **`ProcessPDF`**: Prepares PDF figures for documents and slides: `ProcessPDF fig.pdf [...]` moves the original to `rawpdf/`, rewrites `fig.pdf` with the crop box as the page and all fonts embedded, writes an outlined copy to `olpdf/` and an SVG (sizes in pt) to `svg/`. Needs ghostscript and poppler-utils.
* **`open_root_browser.bat`**: A Windows batch file designed to integrate Windows Explorer with WSL2. It translates Windows file paths natively and registers a handler, allowing you to double-click `.root` files in Windows to instantly launch a WSL-based interactive ROOT `TBrowser`.

---

## Integration

### C++ Source Integration
Include the unified header file in your code:
```cpp
#include <roothelper/roothelper.h>
```

### Installation
```bash
cmake -S src -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build -j
cmake --install build
```
This also installs the scripts in `script/` to `<prefix>/bin`; add it to `PATH` to use them.
It also installs the base 14 substitute fonts in `font/` (URW base 35, the ones ghostscript uses; AGPL-3.0 with a font exception) to `<prefix>/share/roothelper/fonts`, and a fontconfig file that uses them instead of Helvetica, Times, Courier, Symbol and Zapf Dingbats. Enable it with `ln -s <prefix>/share/roothelper/fontconfig/99-roothelper-fonts.conf ~/.config/fontconfig/conf.d/`.

### CMake Setup
Configure your project with `-DCMAKE_PREFIX_PATH=$HOME/.local`, then:
```cmake
find_package(roothelper REQUIRED)
target_link_libraries(YourExecutable PRIVATE roothelper::RootHelper)
```

### ROOT Macros
Library mode (recommended): the installed header loads the compiled libraries itself.
```bash
export ROOT_INCLUDE_PATH=$HOME/.local/include:$ROOT_INCLUDE_PATH
```
```cpp
#include <roothelper/roothelper.h>   // about 0.2 s instead of 1.6 s for source mode
```
For ACLiC (`root macro.C+`), also load the libraries at startup, as in
[src/auxiliary/rootlogon.C](src/auxiliary/rootlogon.C).
Source mode compiles the sources in the interpreter and needs no libraries:
`#include <roothelper/roothelper_source.h>`, or `#include "roothelper.h"` with the
repository root in `ROOT_INCLUDE_PATH`.
