#ifndef ROOTHELPER_USED_IN_INTERPRETER
#include <roothelper/container.h>
#include <roothelper/graphics.h>
#endif

#include <TAxis.h>
#include <TDirectory.h>
#include <TGraph.h>
#include <THStack.h>
#include <TMultiGraph.h>
#include <TObject.h>

#include <stdexcept>
#include <string>

namespace roothelper {

std::vector<std::string> get_object_path_from_directories(
    const std::string& object_name, const std::vector<std::string>& directory_list) {
  std::vector<std::string> path_list;

  for (const auto& directory : directory_list) {
    path_list.emplace_back(directory + "/" + object_name);
  }

  return path_list;
}

ObjectList::ObjectList(const std::string& list_name) : list_name_(list_name) {}

ObjectList::~ObjectList() = default;

MultiObject::MultiObject(MultiObjectType object_type, const std::string& nametitle,
                         TDirectory* directory, const std::vector<std::string>& object_name,
                         const std::string& add_option)
    : object_type_(object_type) {
  initialize_container(nametitle);

  const int n_obj = object_name.size();
  object_.resize(n_obj);

  for (int i_obj = 0; i_obj < n_obj; ++i_obj) {
    TObject* obj = nullptr;
    directory->GetObject(object_name[i_obj].c_str(), obj);
    object_[i_obj] = obj;
    container_->Add(obj, add_option);
  }
}

MultiObject::MultiObject(MultiObjectType object_type, const std::string& nametitle,
                         std::vector<TObject*> obj_list, const std::string& add_option)
    : object_type_(object_type) {
  initialize_container(nametitle);

  const int n_obj = obj_list.size();
  object_.resize(n_obj);

  for (int i_obj = 0; i_obj < n_obj; ++i_obj) {
    TObject* obj = obj_list[i_obj];
    object_[i_obj] = obj;
    container_->Add(obj, add_option);
  }
}

MultiObject::~MultiObject() { delete container_; }

void MultiObject::Draw(std::string option) {
  container_->Draw(option);
  set_axes(container_);
}

void MultiObject::initialize_container(const std::string& nametitle) {
  if (object_type_ == MultiObjectType::Graph) {
    container_ = new ContainerWrapper<TMultiGraph>(nametitle.c_str());
  } else if (object_type_ == MultiObjectType::Histo) {
    container_ = new ContainerWrapper<THStack>(nametitle.c_str());
  } else {
    throw std::runtime_error("unknown type was specified for object type in MultiObject");
  }
}

}  // namespace roothelper
