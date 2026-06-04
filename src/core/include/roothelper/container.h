#ifndef ROOTHELPER_CONTAINER_H_
#define ROOTHELPER_CONTAINER_H_

#include <TAxis.h>
#include <TDirectory.h>
#include <TGraph.h>
#include <TH1.h>
#include <THStack.h>
#include <TList.h>
#include <TMultiGraph.h>
#include <TNamed.h>
#include <TObject.h>

#include <stdexcept>
#include <string>
#include <vector>

#ifndef ROOTHELPER_USED_IN_INTERPRETER
#include <roothelper/graphics.h>
#endif

namespace roothelper {

std::vector<std::string> GetObjectPathFromDirectories(
    const std::string& object_name, const std::vector<std::string>& directory_list);

/**
 * @class ObjectList
 * @brief Manages a collection of loaded ROOT objects from a directory.
 *
 * ObjectList assists in retrieving objects (e.g. histograms, graphs) from a ROOT
 * directory and managing their titles and paths.
 */
class ObjectList {
 public:
  /**
   * @brief Construct a new ObjectList.
   *
   * @param list_name Name of the list.
   */
  explicit ObjectList(const std::string& list_name);
  ~ObjectList();

  /**
   * @brief Load ROOT objects from a directory using a list of path names.
   *
   * @tparam ObjectType The type of objects to retrieve (must inherit from TObject).
   * @param directory The input TDirectory file structure.
   * @param path_list Names of keys/paths to fetch inside the directory.
   * @param title_list Optional list of titles to override the default titles.
   * @return int Number of objects loaded.
   */
  template <class ObjectType>
  int LoadData(TDirectory* directory, const std::vector<std::string>& path_list,
               const std::vector<std::string>& title_list = {});

  /**
   * @brief Get the size of the loaded object list.
   */
  int GetListSize() const { return object_list_.size(); }

  /**
   * @brief Get the list of raw TObject pointers.
   */
  const std::vector<TObject*>& GetObjectList() const { return object_list_; }

  /**
   * @brief Get the list of objects casted to a specific sub-class pointer type.
   */
  template <class ObjectType>
  std::vector<ObjectType*> GetConvertedObjectList() const;

  /**
   * @brief Get a single object casted to a specific sub-class pointer type.
   */
  template <class ObjectType>
  ObjectType* GetObject(int i) const {
    return dynamic_cast<ObjectType*>(object_list_.at(i));
  }

  /**
   * @brief Get the title of the i-th object.
   */
  std::string GetTitle(int i) const { return title_list_.at(i); }

  std::string list_name_;

 private:
  std::vector<TObject*> object_list_;
  std::vector<TDirectory*> directory_list_;
  std::vector<std::string> path_list_;
  std::vector<std::string> title_list_;
};

template <class ObjectType>
int ObjectList::LoadData(TDirectory* directory, const std::vector<std::string>& path_list,
                         const std::vector<std::string>& title_list) {
  const int n_obj = path_list.size();

  for (int i_obj = 0; i_obj < n_obj; ++i_obj) {
    ObjectType* obj_buffer = nullptr;
    directory->GetObject(path_list[i_obj].c_str(), obj_buffer);

    if (obj_buffer == nullptr) {
      throw std::runtime_error(path_list[i_obj] + " was not found");
    }

    object_list_.emplace_back(obj_buffer);
    directory_list_.emplace_back(directory);
    path_list_.emplace_back(path_list[i_obj]);

    if (i_obj < title_list.size()) {
      static_cast<TNamed*>(obj_buffer)->SetTitle(title_list[i_obj].c_str());
      title_list_.emplace_back(title_list[i_obj]);
    } else {
      title_list_.emplace_back(object_list_.back()->GetTitle());
    }
  }

  return n_obj;
}

template <class ObjectType>
std::vector<ObjectType*> ObjectList::GetConvertedObjectList() const {
  std::vector<ObjectType*> converted_object_list;
  for (auto* obj : object_list_) {
    converted_object_list.emplace_back(static_cast<ObjectType*>(obj));
  }
  return converted_object_list;
}

/**
 * @interface IContainerWrapper
 * @brief Abstract interface wrapper for ROOT drawable container types (TMultiGraph and THStack).
 *
 * Provides polymorphic wrapper calls to draw, add elements, get axes limits and references.
 */
struct IContainerWrapper {
  virtual ~IContainerWrapper() = default;

  virtual void Add(TObject* obj, std::string option = "") = 0;
  virtual void Draw(std::string option = "") = 0;
  virtual TAxis* GetXaxis() = 0;
  virtual TAxis* GetYaxis() = 0;
  virtual double GetMinimum() = 0;
  virtual double GetMaximum() = 0;
  virtual std::vector<TObject*> GetObjectList() const = 0;
};

namespace container_wrapper_internal {

template <class ContainerType>
struct DefaultOptions {};

template <>
struct DefaultOptions<TMultiGraph> {
  int GetListSize(TMultiGraph* container) const { return container->GetListOfGraphs()->GetSize(); }
  TList* GetList(TMultiGraph* container) const { return container->GetListOfGraphs(); }
  TGraph* GetTypeSpecifiedObj(TObject* obj) const { return static_cast<TGraph*>(obj); }

  std::string add = "P";
  std::string draw = "A";
};

template <>
struct DefaultOptions<THStack> {
  int GetListSize(THStack* container) const { return container->GetHists()->GetSize(); }
  TList* GetList(THStack* container) const { return container->GetHists(); }
  TH1* GetTypeSpecifiedObj(TObject* obj) const { return static_cast<TH1*>(obj); }

  std::string add = "HIST";
  std::string draw = "NOSTACK";
};

template <class ContainerType>
void SetDefaultAddOptionIfNull(std::string& option) {
  if (option.empty()) {
    DefaultOptions<ContainerType> defaults;
    option = defaults.add;
  }
}

template <class ContainerType>
void SetDefaultDrawOptionIfNull(std::string& option) {
  if (option.empty()) {
    DefaultOptions<ContainerType> defaults;
    option = defaults.draw;
  }
}

}  // namespace container_wrapper_internal

/**
 * @class ContainerWrapper
 * @brief Templated implementation of IContainerWrapper wrapping specific ROOT containers.
 *
 * Typically instantiated as ContainerWrapper<TMultiGraph> or ContainerWrapper<THStack>.
 * Automatically handles automatic color ring assignments and line styles on child additions.
 *
 * @tparam ContainerType The concrete ROOT container type (TMultiGraph or THStack).
 */
template <class ContainerType>
struct ContainerWrapper : IContainerWrapper {
 public:
  explicit ContainerWrapper(const std::string& nametitle);
  ~ContainerWrapper() override;

  void Add(TObject* obj, std::string option) override;
  void Draw(std::string option) override;
  TAxis* GetXaxis() override;
  TAxis* GetYaxis() override;
  double GetMinimum() override;
  double GetMaximum() override;

  std::vector<TObject*> GetObjectList() const override;

  ContainerType* container_;

 private:
  container_wrapper_internal::DefaultOptions<ContainerType> defaults_;
};

template <class ContainerType>
ContainerWrapper<ContainerType>::ContainerWrapper(const std::string& nametitle) {
  container_ = new ContainerType(nametitle.c_str(), nametitle.c_str());
}

template <class ContainerType>
ContainerWrapper<ContainerType>::~ContainerWrapper() {
  delete container_;
}

template <class ContainerType>
void ContainerWrapper<ContainerType>::Add(TObject* obj, std::string option) {
  container_wrapper_internal::SetDefaultAddOptionIfNull<ContainerType>(option);

  auto* specified_obj = defaults_.GetTypeSpecifiedObj(obj);
  container_->Add(specified_obj, option.c_str());
  container_->SetTitle(Form("%s;%s;%s", container_->GetName(),
                            specified_obj->GetXaxis()->GetTitle(),
                            specified_obj->GetYaxis()->GetTitle()));

  const Color_t color = GetColorInRing(defaults_.GetListSize(container_) - 1);
  specified_obj->SetMarkerColor(color);
  specified_obj->SetLineColor(color);
}

template <class ContainerType>
void ContainerWrapper<ContainerType>::Draw(std::string option) {
  container_wrapper_internal::SetDefaultDrawOptionIfNull<ContainerType>(option);
  container_->Draw(option.c_str());
}

template <class ContainerType>
TAxis* ContainerWrapper<ContainerType>::GetXaxis() {
  return container_->GetXaxis();
}

template <class ContainerType>
TAxis* ContainerWrapper<ContainerType>::GetYaxis() {
  return container_->GetYaxis();
}

template <class ContainerType>
double ContainerWrapper<ContainerType>::GetMinimum() {
  return container_->GetHistogram()->GetMinimum();
}

template <class ContainerType>
double ContainerWrapper<ContainerType>::GetMaximum() {
  return container_->GetHistogram()->GetMaximum();
}

template <class ContainerType>
std::vector<TObject*> ContainerWrapper<ContainerType>::GetObjectList() const {
  container_wrapper_internal::DefaultOptions<ContainerType> defaults;
  std::vector<TObject*> object_list;
  TList* list = defaults.GetList(container_);

  for (auto* obj : *list) {
    object_list.emplace_back(obj);
  }

  return object_list;
}

enum class MultiObjectType { Graph, Histo };

/**
 * @class MultiObject
 * @brief Manages plotting multiple combined graphs or histograms.
 *
 * High-level wrapper class that internally instantiates a ContainerWrapper
 * (`TMultiGraph` or `THStack`) depending on the `MultiObjectType` provided,
 * and handles adding and drawing multiple items as a unified group.
 */
class MultiObject {
 public:
  /**
   * @brief Construct a new MultiObject from paths in a TDirectory.
   *
   * @param object_type The container type (MultiObjectType::Graph or MultiObjectType::Histo).
   * @param nametitle Name and title of the container object.
   * @param directory The directory to fetch objects from.
   * @param object_name File paths/keys of the objects to load.
   * @param add_option Default Draw option for elements added to the container.
   */
  MultiObject(MultiObjectType object_type, const std::string& nametitle, TDirectory* directory,
              const std::vector<std::string>& object_name, const std::string& add_option = "");

  /**
   * @brief Construct a new MultiObject from existing TObject pointers.
   *
   * @param object_type The container type (MultiObjectType::Graph or MultiObjectType::Histo).
   * @param nametitle Name and title of the container object.
   * @param obj_list List of existing TObject pointers.
   * @param add_option Default Draw option for elements added to the container.
   */
  MultiObject(MultiObjectType object_type, const std::string& nametitle,
              std::vector<TObject*> obj_list, const std::string& add_option = "");
  ~MultiObject();

  /**
   * @brief Draw the combined objects.
   */
  void Draw(std::string option = "");

  /**
   * @brief Access the underlying ROOT container object (e.g. TMultiGraph or THStack).
   */
  template <class ContainerType>
  ContainerType* get_container() const;

  /**
   * @brief Get the list of individual children objects (casted to their type).
   */
  template <class ObjectType>
  std::vector<ObjectType*> GetObjectList() const;

 private:
  void InitializeContainer(const std::string& nametitle);

  MultiObjectType object_type_;
  IContainerWrapper* container_ = nullptr;
  std::vector<TObject*> object_;
};

template <class ContainerType>
ContainerType* MultiObject::get_container() const {
  return dynamic_cast<ContainerWrapper<ContainerType>*>(container_)->container_;
}

template <class ObjectType>
std::vector<ObjectType*> MultiObject::GetObjectList() const {
  std::vector<ObjectType*> object_list;
  const std::vector<TObject*> abs_obj_list = container_->GetObjectList();

  for (auto* obj : abs_obj_list) {
    object_list.emplace_back(dynamic_cast<ObjectType*>(obj));
  }

  return object_list;
}

}  // namespace roothelper

#endif  // ROOTHELPER_CONTAINER_H_
