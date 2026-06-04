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

## Integration

### C++ Source Integration
Include the unified header file in your code:
```cpp
#include <roothelper/roothelper.h>
```

### CMake Setup
Link your target against `RootHelper`:
```cmake
target_link_libraries(YourExecutable PRIVATE RootHelper)
```
