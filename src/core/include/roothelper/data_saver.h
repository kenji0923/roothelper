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

class DataSaver {
 public:
  explicit DataSaver(const std::filesystem::path& base_directory, bool is_recreate = false);
  ~DataSaver();

  template <class ObjectType>
  void saveObject(ObjectType* obj, const std::filesystem::path& relative_save_directory) const;

  void writeCanvas(TCanvas* c, const std::filesystem::path& relative_save_directory = "") const;

  void write_canvas_without_data_saving(
      TCanvas* c, const std::filesystem::path& relative_save_directory = "") const;

  std::filesystem::path create_directories(const std::filesystem::path& relative_path) const;

 private:
  void createAndChangeDirectory(const std::filesystem::path& relative_save_directory) const;

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
void DataSaver::saveObject(ObjectType* obj,
                            const std::filesystem::path& relative_save_directory) const {
  createAndChangeDirectory(relative_save_directory);

  auto saveChild = [relative_save_directory, this](TList* list) {
    if (list == nullptr) {
      return;
    }

    for (auto* child : *list) {
      saveObject(child, relative_save_directory);
    }
  };

  if (obj->InheritsFrom(TClass::GetClass<TPad>())) {
    if (obj->InheritsFrom(TClass::GetClass<TCanvas>())) {
      obj->Write("", TObject::kOverwrite);
    }
    saveChild(dynamic_cast<TPad*>(obj)->GetListOfPrimitives());
  } else if (obj->InheritsFrom(TClass::GetClass<TMultiGraph>())) {
    obj->Write("", TObject::kOverwrite);
    saveChild(dynamic_cast<TMultiGraph*>(obj)->GetListOfGraphs());
  } else if (obj->InheritsFrom(TClass::GetClass<THStack>())) {
    obj->Write("", TObject::kOverwrite);
    saveChild(dynamic_cast<THStack*>(obj)->GetHists());
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
