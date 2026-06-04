# ROOThelper

`ROOThelper` is a C++ utility library designed to simplify, automate, and beautify CERN ROOT graphics layouts and object management. It handles automatic margins, adaptive title offsets, clean data serialization, and cross-platform WSL integration.

---

## Key Capabilities

### 1. Dynamic & Adaptive Graphics Layouts
ROOT's default layout engine does not adjust margins or title offsets when labels grow or when scientific notation exponents (e.g. $\times 10^3$) appear. `ROOThelper` solves this by introducing dynamic layout rules for axes:
* **Adaptive Y-Axis (`SetYAxis`)**:
  * Scans axis limits and tick step sizes to determine the formatting (decimals and exponents) exactly matching ROOT's rendering.
  * Calculates the exact text width (`GetMaxLabelWidthNdc`) of the longest label.
  * Dynamically expands `gPad->SetLeftMargin` and computes `title_offset` using a constant-gap formula to ensure axis titles never overlap with labels or get clipped at the page boundary.
  * Adjusts the top margin to prevent exponent multipliers at the top of the axis from being cut off.
* **Adaptive Z-Axis/Palette (`SetZAxis`)**:
  * Calculates Z-axis label widths and adjusts `gPad->SetRightMargin` dynamically.
  * Adjusts the vertical color palette coordinates (`TPaletteAxis`) to fit cleanly in the margin.
  * Adjusts the Z-axis title offset to maintain a constant visual gap from the labels.
* **Aspect Ratio-Independent Spacing**:
  * Title offsets automatically compensate for canvas aspect ratios ($W/H$), guaranteeing consistent visual margins regardless of window shape.

---

### 2. High-Quality Size Presets
* Provides standard sizing templates matching publication-quality fonts:
  * `g_size_8pt` (optimised for standard 8pt paper layouts, $700 \times 500$ px).
  * `g_size_10pt` (optimised for standard 10pt paper layouts, $700 \times 500$ px).
* Easily switches layout parameters and spacing buffers globally.

---

### 3. Structured Data Serialization (`DataSaver`)
* **Unified Export**: Serializes canvases directly to both `.pdf` and `.png` formats simultaneously while saving original C++ ROOT objects into a single `.root` file.
* **Automated Data Organization**:
  * Recursively traverses drawn objects in the canvas (including primitives, sub-pads, stacks, and multi-graphs).
  * Automatically creates nested subdirectories (`TDirectory`) in the output `.root` file to keep the root directory structured:
    * `TCanvas` named `c_xxx` $\rightarrow$ primitives saved in `data_c_xxx/`
    * `TMultiGraph` named `mg_xxx` $\rightarrow$ graphs saved in `data_mg_xxx/`
    * `THStack` named `hs_xxx` $\rightarrow$ histograms saved in `data_hs_xxx/`

---

### 4. Color & Style Cycles
* Helper utilities like `GetColorInRing` provide predefined color palettes (Azure, Orange, Teal, Magenta, Cyan) for easy color rotation inside multi-graphs and stacked histograms.

---

### 5. Windows-WSL Shell Integration
* Includes a helper script [open_root_browser.bat](file:///home/kshu/work/development/ROOT_helper/script/open_root_browser.bat) that integrates Windows Explorer with WSL2:
  * Translates paths natively between Windows format (including local drives and UNC network paths like `\\wsl.localhost\`) and Linux format using `WSLENV`.
  * Allows you to **double-click a `.root` file in Windows Explorer** to instantly open a WSL-native ROOT `TBrowser` window on your desktop using WSLg.

---

## Library Usage

### In C++ / ROOT Macros
Include the main header:
```cpp
#include <roothelper/roothelper.h>
```

### In CMake Projects
Link your target against `ROOThelper`:
```cmake
target_link_libraries(MyExecutable PRIVATE ROOThelper)
```
