#include "MockBuilder.hpp"

#include "ElementMock.hpp"

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

void MockBuilder::setDeviceInfo(
    const string& unique_id, const BuildInfo& element_info) {
  if (!result_) {
    result_ = make_unique<NiceMock<DeviceMock>>(
        unique_id, FullMetaInfo{element_info.name, element_info.description});
  } else {
    throw DeviceBuildInProgress();
  }
}

string MockBuilder::addGroup(const BuildInfo& element_info) {
  return addGroup("", element_info);
}

string MockBuilder::addGroup(
    const string& parent_id, const BuildInfo& element_info) {
  checkBase();

  string id;
  if (parent_id.empty()) {
    id = result_->generateID();
  } else {
    auto subgroup = getParentGroup(parent_id);
    id = subgroup->generateID();
  }

  auto group = make_shared<NiceMock<GroupMock>>(id);
  subgroups_.try_emplace(id, group);
  addElementMock(group, id, element_info);
  return id;
}

string MockBuilder::addReadable(const BuildInfo& element_info,
    DataType data_type,
    const ReadCallback& read_cb) {
  return addReadable("", element_info, data_type, read_cb);
}

string MockBuilder::addReadable(const string& parent_id,
    const BuildInfo& element_info,
    DataType data_type,
    const ReadCallback& read_cb) {
  checkBase();
  if (data_type == DataType::None || data_type == DataType::Unknown) {
    throw invalid_argument("Data Type can not be None or Unknown");
  }
  if (!read_cb) {
    throw invalid_argument("ReadCallback can not be nullptr");
  }

  string id;
  if (parent_id.empty()) {
    id = result_->generateID();
  } else {
    auto subgroup = getParentGroup(parent_id);
    id = subgroup->generateID();
  }

  auto readable = make_shared<NiceMock<ReadableMock>>(data_type, read_cb);
  addElementMock(readable, id, element_info);
  return id;
}

string MockBuilder::addWritable(const BuildInfo& element_info,
    DataType data_type,
    const WriteCallback& write_cb,
    const ReadCallback& read_cb) {
  return addWritable("", element_info, data_type, write_cb, read_cb);
}

string MockBuilder::addWritable(const string& parent_id,
    const BuildInfo& element_info,
    DataType data_type,
    const WriteCallback& write_cb,
    const ReadCallback& read_cb) {
  checkBase();
  if (data_type == DataType::None || data_type == DataType::Unknown) {
    throw invalid_argument("Data Type can not be None or Unknown");
  }
  if (!write_cb) {
    throw invalid_argument("WriteCallback can not be nullptr");
  }

  string id;
  if (parent_id.empty()) {
    id = result_->generateID();
  } else {
    auto subgroup = getParentGroup(parent_id);
    id = subgroup->generateID();
  }

  auto writable =
      make_shared<NiceMock<WritableMock>>(data_type, read_cb, write_cb);
  addElementMock(writable, id, element_info);
  return id;
}

pair<string, MockBuilder::NotifyCallback> MockBuilder::addObservable(
    const BuildInfo& element_info,
    DataType data_type,
    const ReadCallback& read_cb,
    const IsObservingCallback& observe_cb) {
  return addObservable("", element_info, data_type, read_cb, observe_cb);
}

pair<string, MockBuilder::NotifyCallback> MockBuilder::addObservable(
    const string& parent_id,
    const BuildInfo& element_info,
    DataType data_type,
    const ReadCallback& read_cb,
    const IsObservingCallback& observe_cb) {
  checkBase();
  if (data_type == DataType::None || data_type == DataType::Unknown) {
    throw invalid_argument("Data Type can not be None or Unknown");
  }
  if (!read_cb) {
    throw invalid_argument("ReadCallback can not be nullptr");
  }
  if (!observe_cb) {
    throw invalid_argument("IsObservingCallback can not be nullptr");
  }

  string id;
  if (parent_id.empty()) {
    id = result_->generateID();
  } else {
    auto subgroup = getParentGroup(parent_id);
    id = subgroup->generateID();
  }

  auto observable = make_shared<NiceMock<ObservableMock>>(data_type, read_cb);
  observable->enableSubscribeFaking(observe_cb);
  addElementMock(observable, id, element_info);
  return make_pair(
      id, bind(&ObservableMock::notify, observable, placeholders::_1));
}

string MockBuilder::addCallable(const BuildInfo& element_info,
    const ExecuteCallback& execute_cb,
    const ParameterTypes& parameter_types) {
  return addCallable("", element_info, execute_cb, parameter_types);
}

string MockBuilder::addCallable(const string& parent_id,
    const BuildInfo& element_info,
    const ExecuteCallback& execute_cb,
    const ParameterTypes& parameter_types) {
  checkBase();
  if (!execute_cb) {
    throw invalid_argument("ExecuteCallback can not be nullptr");
  }

  string id;
  if (parent_id.empty()) {
    id = result_->generateID();
  } else {
    auto subgroup = getParentGroup(parent_id);
    id = subgroup->generateID();
  }

  auto callable =
      make_shared<NiceMock<CallableMock>>(execute_cb, parameter_types);
  addElementMock(callable, id, element_info);
  return id;
}

string MockBuilder::addCallable(const BuildInfo& element_info,
    DataType result_type,
    const ExecuteCallback& execute_cb,
    const AsyncExecuteCallback& async_execute_cb,
    const CancelCallback& cancel_cb,
    const ParameterTypes& parameter_types) {
  return addCallable("",
      element_info,
      result_type,
      execute_cb,
      async_execute_cb,
      cancel_cb,
      parameter_types);
}

string MockBuilder::addCallable(const string& parent_id,
    const BuildInfo& element_info,
    DataType result_type,
    const ExecuteCallback& execute_cb,
    const AsyncExecuteCallback& async_execute_cb,
    const CancelCallback& cancel_cb,
    const ParameterTypes& parameter_types) {
  checkBase();
  if (result_type == DataType::None || result_type == DataType::Unknown) {
    throw invalid_argument("Result Type can not be None or Unknown");
  }
  if (!execute_cb) {
    throw invalid_argument("ExecuteCallback can not be nullptr");
  }
  if (!async_execute_cb) {
    throw invalid_argument("AsyncExecuteCallback can not be nullptr");
  }
  if (!cancel_cb) {
    throw invalid_argument("CancelCallback can not be nullptr");
  }

  string id;
  if (parent_id.empty()) {
    id = result_->generateID();
  } else {
    auto subgroup = getParentGroup(parent_id);
    id = subgroup->generateID();
  }

  auto callable = make_shared<NiceMock<CallableMock>>(
      result_type, execute_cb, async_execute_cb, cancel_cb, parameter_types);
  addElementMock(callable, id, element_info);
  return id;
}

void MockBuilder::addElementMock(const ElementFunction& function,
    const std::string& id,
    const BuildInfo& element_info) {
  auto element = make_shared<NiceMock<ElementMock>>(
      function, id, FullMetaInfo{element_info.name, element_info.description});
  result_->addElement(element);
}

GroupMockPtr MockBuilder::getParentGroup(const string& parent_id) {
  if (auto it = subgroups_.find(parent_id); it != subgroups_.end()) {
    return it->second;
  } else {
    throw invalid_argument("No parent group with ID " + parent_id +
        " exists. Build parent group first");
  }
}

void MockBuilder::checkBase() const {
  if (!result_) {
    throw DeviceInfoNotSet();
  }
}

void MockBuilder::checkGroups() const {
  if (result_->group()->size() == 0) {
    throw GroupEmpty(result_->id());
  }
  for (const auto& [id, group] : subgroups_) {
    if (group->size() == 0) {
      throw GroupEmpty(result_->id(), id);
    }
  }
}

unique_ptr<Device> MockBuilder::result() {
  checkBase();
  checkGroups();
  subgroups_.clear();
  return move(result_);
}

} // namespace Information_Model::testing
