# API Reference Guide - roothelper

Welcome to the `roothelper` C++ API reference manual. This library provides helper classes and utilities to simplify plotting, formatting, and saving CERN ROOT object data.

All classes and functions are located inside the `roothelper` namespace.

---

## 1. Analysis Submodule (`analysis.h`)

Provides arithmetic and layout conversion utilities for histograms and graphs.

### Functions

```cpp
TH1* ScaleHistoX(TH1* h, double scale);
```
* **Purpose**: Scales all X-axis bin edges of a histogram by a constant factor and returns a new histogram. Useful for unit conversions (e.g., seconds to milliseconds).
* **Parameters**:
  * `h`: Pointer to the input histogram.
  * `scale`: Scaling factor.
* **Returns**: Pointer to the newly created scaled histogram (caller takes ownership).

```cpp
TH1* ConvertToDensityHisto(TH1* h);
```
* **Purpose**: Divides each bin content and its error by the bin width to convert the histogram into a density plot. Useful for comparing plots with variable/non-uniform binning.
* **Parameters**:
  * `h`: Pointer to the input histogram.
  * **Returns**: Pointer to the density histogram (caller takes ownership).

```cpp
TGraphErrors* GetGraphG0xAPlusG1(double a, const TGraphErrors* g0, const TGraphErrors* g1);
```
* **Purpose**: Computes the linear combination `Y_new = a * Y_0 + Y_1` point-by-point, propagating the errors on the Y-values. Assumes `g0` and `g1` share identical X coordinates.
* **Parameters**:
  * `a`: Scaling factor for the first graph's Y values.
  * `g0`: First input graph.
  * `g1`: Second input graph.
  * **Returns**: Pointer to the combined `TGraphErrors` (caller takes ownership).

---

## 2. Container Submodule (`container.h`)

Provides classes to load lists of keys from ROOT files, wrap them in polymorphic interfaces, and plot them in groups.

### `ObjectList`

Helper class to load lists of objects dynamically from a `TDirectory`.

```cpp
explicit ObjectList(const std::string& list_name);
```
* **Purpose**: Construct an `ObjectList` with a given name.

```cpp
template <class ObjectType>
int LoadData(TDirectory* directory, const std::vector<std::string>& path_list, const std::vector<std::string>& title_list = {});
```
* **Purpose**: Load objects from a directory using a vector of relative paths, optionally overriding titles. Returns the number of loaded objects.

```cpp
int GetListSize() const;
const std::vector<TObject*>& GetObjectList() const;
```
* **Purpose**: Retrieve the size or raw list of loaded `TObject` pointers.

---

### `IContainerWrapper` & `ContainerWrapper`

`IContainerWrapper` is an abstract interface wrapper that represents ROOT container objects (`TMultiGraph` or `THStack`). `ContainerWrapper` is its templated implementation.

```cpp
template <class ContainerType>
struct ContainerWrapper : IContainerWrapper;
```
* **Usage**: Typically instantiated as `ContainerWrapper<TMultiGraph>` or `ContainerWrapper<THStack>`. It automatically assigns matching color rings and styles as child objects are added to the container.

---

### `MultiObject`

High-level wrapper that groups multiple graphs or histograms and handles drawing them as a single object.

```cpp
MultiObject(MultiObjectType object_type, const std::string& nametitle, TDirectory* directory, const std::vector<std::string>& object_name, const std::string& add_option = "");
```
* **Purpose**: Loads the list of files from `directory` and initializes a `TMultiGraph` or `THStack`.

```cpp
void Draw(std::string option = "");
```
* **Purpose**: Draws the combined object to the active pad.

---

## 3. DataSaver Submodule (`data_saver.h`)

Coordinates plotting canvas images to disk and saving native C++ objects in a structured `.root` file.

### `DataSaver`

```cpp
explicit DataSaver(const std::filesystem::path& base_directory, bool is_recreate = false);
```
* **Purpose**: Creates the base output directory and opens the output file `data.root`.

```cpp
void WriteCanvas(TCanvas* c, const std::filesystem::path& relative_save_directory = "") const;
```
* **Purpose**: 
  1. Prints the canvas to disk as `PDF` and `PNG`.
  2. Recursively calls `SaveObject` to write the canvas and its contents into `data.root`.

### Subdirectory Storage Structure
To prevent cluttering, child objects drawn inside canvases or container variables are automatically saved in separate, nested directories:
* **TCanvas** `c_xxx` -> Children saved in `data_c_xxx/`
* **TMultiGraph** `mg_xxx` -> Children saved in `data_mg_xxx/`
* **THStack** `hs_xxx` -> Children saved in `data_hs_xxx/`

---

## 4. Graphics Submodule (`graphics.h`)

Manages layout dimensions, canvas formatting, and dynamic margin optimization.

### Styling & Axis Solvers

```cpp
void Prepare();
```
* **Purpose**: Sets up global styles, custom color schemes, and font configurations.

```cpp
TCanvas* CreateCanvas(const std::string& name, const std::string& title, unsigned int n_pad_x = 1, unsigned int n_pad_y = 1, ...);
```
* **Purpose**: Creates a customized canvas divided into grid coordinates.

```cpp
void SetAxes(GraphType* graph_object, Option_t* draw_option = "");
```
* **Purpose**: High-level routine that styles the X, Y, and Z axes on a graph/histogram and calls layout optimizations.

```cpp
void OptimizeYAxisLayout(TAxis* y_axis);
```
* **Purpose**: Solves the Y-axis label width dynamically and applies a constant-gap offset formula:
  `title_offset = (distance_from_axis / title_size) * 0.60 + 0.20`
  This ensures the visual gap between labels and the Y-axis title remains perfectly consistent.

---

## 5. Utility Scripts (`script/`)

The repository includes helper scripts to streamline the development cycle and integrate WSL with Windows environments.

### `rooti` (Python Wrapper for Macro Runs)
A CLI script that automates type inference and formatting for ROOT macro parameters.
* **Usage**: `rooti [-nb] [-nq] <script.cpp> [macro_args...]`
* **Features**:
  * Automatically detects and formats booleans (`true`/`false`), numeric types, braced initializer lists (e.g. `{1.0, 2.0}`), and strings (wrapping them in quotes and escaping internal quotes).
  * Executes the command using `root -b -q 'script.cpp(args)'` by default.
  * `-nb`: Do not suppress batch mode (runs without `-b`).
  * `-nq`: Do not quit after macro execution (runs without `-q`).

### `CreateRootClangd` (Language Server Protocol Setup)
A bash helper to configure `.clangd` local compiler flags for code intelligence.
* **Usage**: Run `./CreateRootClangd` in the project root directory.
* **Features**:
  * Creates a local `.clangd` file if it does not exist.
  * Retrieves compile flags from `root-config --cflags` and adds them to `.clangd`, allowing modern language servers (e.g. clangd) to correctly parse ROOT headers and symbols.

### `open_root_browser.bat` (Windows-WSL File Association)
A Windows Batch file mapping local files to the WSL environment.
* **Usage**: Associate `.root` files in Windows Explorer to open using this batch script.
* **Features**:
  * Automatically translates UNC and local paths using `WSLENV` path translation flags (`/p`).
  * Launches the interactive `TBrowser` via WSLg using `wsl bash -lc "root -l \"$ROOT_FILE_PATH\" -e \"new TBrowser\""`.

