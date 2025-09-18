#ifndef __STAG_INFORMATION_MODEL_MOCKS_MOCK_BUILDER_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_MOCK_BUILDER_HPP
#include "DeviceMock.hpp"

#include <Information_Model/DeviceBuilder.hpp>

namespace Information_Model::testing {

struct MockBuilder : public DeviceBuilder {
  void setDeviceInfo(
      const std::string& unique_id, const BuildInfo& element_info) final;

  std::string addGroup(const BuildInfo& element_info) final;

  std::string addGroup(
      const std::string& parent_id, const BuildInfo& element_info) final;

  std::string addReadable(const BuildInfo& element_info, DataType data_type,
      const ReadCallback& read_cb) final;

  std::string addReadable(const std::string& parent_id,
      const BuildInfo& element_info, DataType data_type,
      const ReadCallback& read_cb) final;

  std::string addWritable(const BuildInfo& element_info, DataType data_type,
      const WriteCallback& write_cb,
      const ReadCallback& read_cb = nullptr) final;

  std::string addWritable(const std::string& parent_id,
      const BuildInfo& element_info, DataType data_type,
      const WriteCallback& write_cb,
      const ReadCallback& read_cb = nullptr) final;

  std::pair<std::string, NotifyCallback> addObservable(
      const BuildInfo& element_info, DataType data_type,
      const ReadCallback& read_cb, const IsObservingCallback& observe_cb) final;

  std::pair<std::string, NotifyCallback> addObservable(
      const std::string& parent_id, const BuildInfo& element_info,
      DataType data_type, const ReadCallback& read_cb,
      const IsObservingCallback& observe_cb) final;

  std::string addCallable(const BuildInfo& element_info,
      const ExecuteCallback& execute_cb,
      const ParameterTypes& parameter_types = {}) final;

  std::string addCallable(const std::string& parent_id,
      const BuildInfo& element_info, const ExecuteCallback& execute_cb,
      const ParameterTypes& parameter_types = {}) final;

  std::string addCallable(const BuildInfo& element_info, DataType result_type,
      const ExecuteCallback& execute_cb,
      const AsyncExecuteCallback& async_execute_cb,
      const CancelCallback& cancel_cb,
      const ParameterTypes& parameter_types = {}) final;

  std::string addCallable(const std::string& parent_id,
      const BuildInfo& element_info, DataType result_type,
      const ExecuteCallback& execute_cb,
      const AsyncExecuteCallback& async_execute_cb,
      const CancelCallback& cancel_cb,
      const ParameterTypes& parameter_types = {}) final;

  std::unique_ptr<Device> result() final;

private:
  GroupMockPtr getParentGroup(const std::string& parent_id);

  void addElementMock(const ElementFunction& function, const std::string& id,
      const BuildInfo& element_info);

  void checkBase() const;
  void checkGroups() const;

  std::unique_ptr<DeviceMock> result_;
  std::unordered_map<std::string, GroupMockPtr> subgroups_;
};

using MockBuilderPtr = std::shared_ptr<MockBuilder>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_MOCK_BUILDER_HPP