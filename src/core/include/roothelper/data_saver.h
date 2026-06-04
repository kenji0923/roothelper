#ifndef ROOTHELPER_DATA_SAVER_H_
#define ROOTHELPER_DATA_SAVER_H_

#include <TCanvas.h>
#include <TClass.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TH1.h>
#include <THStack.h>
#include <TList.h>
#include <TMultiGraph.h>
#include <TPad.h>

#include <filesystem>
#include <memory>
#include <vector>

namespace roothelper {

/**
 * @class DataSaver
 * @brief Manages saving graphical plots (PDF/PNG) and native ROOT objects to disk and a ROOT TFile.
 *
 * DataSaver encapsulates output directories and a single output ROOT file (`data.root`),
 * providing unified methods to write canvases to disk and recursively save drawable
 * ROOT primitives (histograms, graphs, stacks, multi-graphs) inside nested directories.
 */
class DataSaver {
 public:
  /**
   * @brief Construct a new DataSaver.
   *
   * @param base_directory Path to the directory where PDF/PNG plots and data.root will be stored.
   * @param is_recreate If true, recreates the target ROOT file. Otherwise updates it in place.
   */
  explicit DataSaver(const std::filesystem::path& base_directory, bool is_recreate = false);

  /**
   * @brief Destroy the DataSaver, saving and closing the open ROOT file.
   */
  ~DataSaver();

  /**
   * @brief Recursively save a ROOT object and its child primitives.
   *
   * This is a template function that automatically matches ROOT container objects.
   * Children are grouped into sub-directories to keep the structure clean:
   * - TCanvas `c_xxx` children are written under `data_c_xxx/`
   * - TMultiGraph `mg_xxx` children are written under `data_mg_xxx/`
   * - THStack `hs_xxx` children are written under `data_hs_xxx/`
   *
   * @tparam ObjectType The type of the object (must inherit from TObject).
   * @param obj Pointer to the object to write.
   * @param relative_save_directory Target subdirectory inside the ROOT TFile.
   */
  template <class ObjectType>
  void SaveObject(ObjectType* obj, const std::filesystem::path& relative_save_directory) const;

  /**
   * @brief Write a canvas to disk as PDF and PNG, and recursively save its contents to the ROOT
   * TFile.
   *
   * @param c Pointer to the TCanvas to save.
   * @param relative_save_directory Path suffix on disk and inside the ROOT TFile.
   */
  void WriteCanvas(TCanvas* c, const std::filesystem::path& relative_save_directory = "") const;

  /**
   * @brief Print the canvas to disk as PDF and PNG without saving its data to the ROOT file.
   *
   * @param c Pointer to the TCanvas to print.
   * @param relative_save_directory Directory suffix where plots are printed.
   */
  void write_canvas_without_data_saving(
      TCanvas* c, const std::filesystem::path& relative_save_directory = "") const;

  /**
   * @brief Create a filesystem path relative to the base output directory.
   *
   * @param relative_path Suffix path.
   * @return std::filesystem::path Absolute created path.
   */
  std::filesystem::path create_directories(const std::filesystem::path& relative_path) const;

 private:
  void CreateAndChangeDirectory(const std::filesystem::path& relative_save_directory) const;

  const std::filesystem::path base_directory_;
  std::unique_ptr<TFile> f_write_;
  std::vector<TClass*> class_to_save_list_ = {
      TClass::GetClass<TH1>(),
      TClass::GetClass<TGraph>(),
      TClass::GetClass<TGraph2D>(),
      TClass::GetClass<TMultiGraph>(),
  };
};

template <class ObjectType>
void DataSaver::SaveObject(ObjectType* obj,
                           const std::filesystem::path& relative_save_directory) const {
  CreateAndChangeDirectory(relative_save_directory);

  auto SaveChild = [this](TList* list, const std::filesystem::path& child_dir) {
    if (list == nullptr) {
      return;
    }

    for (auto* child : *list) {
      SaveObject(child, child_dir);
    }
  };

  if (obj->InheritsFrom(TClass::GetClass<TPad>())) {
    if (obj->InheritsFrom(TClass::GetClass<TCanvas>())) {
      obj->Write("", TObject::kOverwrite);
      SaveChild(dynamic_cast<TPad*>(obj)->GetListOfPrimitives(),
                relative_save_directory / ("data_" + std::string(obj->GetName())));
    } else {
      SaveChild(dynamic_cast<TPad*>(obj)->GetListOfPrimitives(), relative_save_directory);
    }
  } else if (obj->InheritsFrom(TClass::GetClass<TMultiGraph>())) {
    obj->Write("", TObject::kOverwrite);
    SaveChild(dynamic_cast<TMultiGraph*>(obj)->GetListOfGraphs(),
              relative_save_directory / ("data_" + std::string(obj->GetName())));
  } else if (obj->InheritsFrom(TClass::GetClass<THStack>())) {
    obj->Write("", TObject::kOverwrite);
    SaveChild(dynamic_cast<THStack*>(obj)->GetHists(),
              relative_save_directory / ("data_" + std::string(obj->GetName())));
  } else {
    for (const auto* class_type : class_to_save_list_) {
      if (obj->InheritsFrom(class_type)) {
        obj->Write("", TObject::kOverwrite);
        break;
      }
    }
  }
}

}  // namespace roothelper

#endif  // ROOTHELPER_DATA_SAVER_H_
