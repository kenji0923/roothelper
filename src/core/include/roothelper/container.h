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

std::vector<std::string> getObjectPathFromDirectories(
    const std::string& object_name, const std::vector<std::string>& directory_list);

class ObjectList {
 public:
  explicit ObjectList(const std::string& list_name);
  ~ObjectList();

  template <class ObjectType>
  int loadData(TDirectory* directory, const std::vector<std::string>& path_list,
                const std::vector<std::string>& title_list = {});

  int getListSize() const { return object_list_.size(); }
  const std::vector<TObject*>& getObjectList() const { return object_list_; }

  template <class ObjectType>
  std::vector<ObjectType*> getConvertedObjectList() const;

  template <class ObjectType>
  ObjectType* getObject(int i) const {
    return dynamic_cast<ObjectType*>(object_list_.at(i));
  }

  std::string getTitle(int i) const { return title_list_.at(i); }

  std::string list_name_;

 private:
  std::vector<TObject*> object_list_;
  std::vector<TDirectory*> directory_list_;
  std::vector<std::string> path_list_;
  std::vector<std::string> title_list_;
};

template <class ObjectType>
int ObjectList::loadData(TDirectory* directory, const std::vector<std::string>& path_list,
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
std::vector<ObjectType*> ObjectList::getConvertedObjectList() const {
  std::vector<ObjectType*> converted_object_list;
  for (auto* obj : object_list_) {
    converted_object_list.emplace_back(static_cast<ObjectType*>(obj));
  }
  return converted_object_list;
}

struct IContainerWrapper {
  virtual ~IContainerWrapper() = default;

  virtual void Add(TObject* obj, std::string option = "") = 0;
  virtual void Draw(std::string option = "") = 0;
  virtual TAxis* GetXaxis() = 0;
  virtual TAxis* GetYaxis() = 0;
  virtual double GetMinimum() = 0;
  virtual double GetMaximum() = 0;
  virtual std::vector<TObject*> getObjectList() const = 0;
};

namespace container_wrapper_internal {

template <class ContainerType>
struct DefaultOptions {};

template <>
struct DefaultOptions<TMultiGraph> {
  int getListSize(TMultiGraph* container) const {
    return container->GetListOfGraphs()->GetSize();
  }
  TList* getList(TMultiGraph* container) const { return container->GetListOfGraphs(); }
  TGraph* getTypeSpecifiedObj(TObject* obj) const { return static_cast<TGraph*>(obj); }

  std::string add = "P";
  std::string draw = "A";
};

template <>
struct DefaultOptions<THStack> {
  int getListSize(THStack* container) const { return container->GetHists()->GetSize(); }
  TList* getList(THStack* container) const { return container->GetHists(); }
  TH1* getTypeSpecifiedObj(TObject* obj) const { return static_cast<TH1*>(obj); }

  std::string add = "HIST";
  std::string draw = "NOSTACK";
};

template <class ContainerType>
void setDefaultAddOptionIfNull(std::string& option) {
  if (option.empty()) {
    DefaultOptions<ContainerType> defaults;
    option = defaults.add;
  }
}

template <class ContainerType>
void setDefaultDrawOptionIfNull(std::string& option) {
  if (option.empty()) {
    DefaultOptions<ContainerType> defaults;
    option = defaults.draw;
  }
}

}  // namespace container_wrapper_internal

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

  std::vector<TObject*> getObjectList() const override;

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
  container_wrapper_internal::setDefaultAddOptionIfNull<ContainerType>(option);

  auto* specified_obj = defaults_.getTypeSpecifiedObj(obj);
  container_->Add(specified_obj, option.c_str());
  container_->SetTitle(Form("%s;%s;%s", container_->GetName(),
                            specified_obj->GetXaxis()->GetTitle(),
                            specified_obj->GetYaxis()->GetTitle()));

  const Color_t color = getColorInRing(defaults_.getListSize(container_) - 1);
  specified_obj->SetMarkerColor(color);
  specified_obj->SetLineColor(color);
}

template <class ContainerType>
void ContainerWrapper<ContainerType>::Draw(std::string option) {
  container_wrapper_internal::setDefaultDrawOptionIfNull<ContainerType>(option);
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
std::vector<TObject*> ContainerWrapper<ContainerType>::getObjectList() const {
  container_wrapper_internal::DefaultOptions<ContainerType> defaults;
  std::vector<TObject*> object_list;
  TList* list = defaults.getList(container_);

  for (auto* obj : *list) {
    object_list.emplace_back(obj);
  }

  return object_list;
}

enum class MultiObjectType { Graph, Histo };

class MultiObject {
 public:
  MultiObject(MultiObjectType object_type, const std::string& nametitle, TDirectory* directory,
              const std::vector<std::string>& object_name, const std::string& add_option = "");
  MultiObject(MultiObjectType object_type, const std::string& nametitle,
              std::vector<TObject*> obj_list, const std::string& add_option = "");
  ~MultiObject();

  void Draw(std::string option = "");

  template <class ContainerType>
  ContainerType* get_container() const;

  template <class ObjectType>
  std::vector<ObjectType*> getObjectList() const;

 private:
  void initializeContainer(const std::string& nametitle);

  MultiObjectType object_type_;
  IContainerWrapper* container_ = nullptr;
  std::vector<TObject*> object_;
};

template <class ContainerType>
ContainerType* MultiObject::get_container() const {
  return dynamic_cast<ContainerWrapper<ContainerType>*>(container_)->container_;
}

template <class ObjectType>
std::vector<ObjectType*> MultiObject::getObjectList() const {
  std::vector<ObjectType*> object_list;
  const std::vector<TObject*> abs_obj_list = container_->getObjectList();

  for (auto* obj : abs_obj_list) {
    object_list.emplace_back(dynamic_cast<ObjectType*>(obj));
  }

  return object_list;
}

}  // namespace roothelper

#endif  // ROOTHELPER_CONTAINER_H_
