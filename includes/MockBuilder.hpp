#ifndef __STAG_INFORMATION_MODEL_MOCKS_MOCK_BUILDER_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_MOCK_BUILDER_HPP
#include "DeviceMock.hpp"
#include "FakeExecutor.hpp"

#include <Information_Model/DeviceBuilder.hpp>

namespace Information_Model::testing {

struct MockBuilder : public DeviceBuilder {
  void setDeviceInfo(
      const std::string& unique_id, const BuildInfo& element_info) final;

  std::string addGroup(const BuildInfo& element_info) final;

  std::string addGroup(
      const std::string& parent_id, const BuildInfo& element_info) final;

  /**
   * @brief Helper method to create default mock readable
   *
   */
  std::string addReadable(const BuildInfo& element_info, DataType data_type);

  /**
   * @brief Helper method to create mock readable that always returns the given
   * value
   *
   */
  std::string addReadable(
      const BuildInfo& element_info, const DataVariant& default_value);

  std::string addReadable(const BuildInfo& element_info, DataType data_type,
      const ReadCallback& read_cb) final;

  std::string addReadable(const std::string& parent_id,
      const BuildInfo& element_info, DataType data_type,
      const ReadCallback& read_cb) final;

  /**
   * @brief Helper method to create default mock readable within a given
   * subgroup
   *
   */
  std::string addReadable(const std::string& parent_id,
      const BuildInfo& element_info, DataType data_type);

  /**
   * @brief Helper method to create mock readable that always returns the given
   * value within a given subgroup
   *
   */
  std::string addReadable(const std::string& parent_id,
      const BuildInfo& element_info, const DataVariant& default_value);

  /**
   * @brief Helper method to create default write-only writable
   *
   */
  std::string addWritable(const BuildInfo& element_info, DataType data_type);

  /**
   * @brief Helper method to create read-and-writable mock that always returns
   * the given value
   *
   */
  std::string addWritable(
      const BuildInfo& element_info, const DataVariant& default_value);

  std::string addWritable(const BuildInfo& element_info, DataType data_type,
      const WriteCallback& write_cb,
      const ReadCallback& read_cb = nullptr) final;

  std::string addWritable(const std::string& parent_id,
      const BuildInfo& element_info, DataType data_type,
      const WriteCallback& write_cb,
      const ReadCallback& read_cb = nullptr) final;

  /**
   * @brief Helper method to create default write-only writable within a given
   * subgroup within a given subgroup
   *
   *
   */
  std::string addWritable(const std::string& parent_id,
      const BuildInfo& element_info, DataType data_type);

  /**
   * @brief Helper method to create read-and-writable mock that always returns
   * the given value within a given subgroup
   *
   *
   */
  std::string addWritable(const std::string& parent_id,
      const BuildInfo& element_info, const DataVariant& default_value);

  /**
   * @brief Helper method to create default mock observable
   *
   */
  std::pair<std::string, NotifyCallback> addObservable(
      const BuildInfo& element_info, DataType data_type,
      const IsObservingCallback& observe_cb);

  /**
   * @brief Helper method to create mock observable that always returns the
   * given value on read() call
   *
   */
  std::pair<std::string, NotifyCallback> addObservable(
      const BuildInfo& element_info, const DataVariant& default_value,
      const IsObservingCallback& observe_cb);

  std::pair<std::string, NotifyCallback> addObservable(
      const BuildInfo& element_info, DataType data_type,
      const ReadCallback& read_cb, const IsObservingCallback& observe_cb) final;

  std::pair<std::string, NotifyCallback> addObservable(
      const std::string& parent_id, const BuildInfo& element_info,
      DataType data_type, const ReadCallback& read_cb,
      const IsObservingCallback& observe_cb) final;

  /**
   * @brief Helper method to create default mock observable within a given
   * subgroup
   *
   */
  std::pair<std::string, NotifyCallback> addObservable(
      const std::string& parent_id, const BuildInfo& element_info,
      DataType data_type, const IsObservingCallback& observe_cb);

  /**
   * @brief Helper method to create mock observable that always returns the
   * given value on read() call within a given subgroup
   *
   */
  std::pair<std::string, NotifyCallback> addObservable(
      const std::string& parent_id, const BuildInfo& element_info,
      const DataVariant& default_value, const IsObservingCallback& observe_cb);

  /**
   * @brief Helper method to create default mock callable, that uses the
   * internal executor
   *
   */
  std::string addCallable(const BuildInfo& element_info,
      const ParameterTypes& parameter_types = {});

  std::string addCallable(const BuildInfo& element_info,
      const ExecuteCallback& execute_cb,
      const ParameterTypes& parameter_types = {}) final;

  std::string addCallable(const std::string& parent_id,
      const BuildInfo& element_info, const ExecuteCallback& execute_cb,
      const ParameterTypes& parameter_types = {}) final;

  /**
   * @brief Helper method to create default mock callable, that uses the
   * internal executor
   *
   */
  std::string addCallable(const BuildInfo& element_info, DataType result_type,
      const ParameterTypes& parameter_types = {});

  /**
   * @brief Helper method to create a mock callable, that uses
   * a given executor
   *
   */
  std::string addCallable(
      const BuildInfo& element_info, const ExecutorPtr& executor);

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

  /**
   * @brief Helper method to create default mock callable, that uses the
   * internal executor
   *
   */
  std::string addCallable(const std::string& parent_id,
      const BuildInfo& element_info, DataType result_type,
      const ParameterTypes& parameter_types = {});

  /**
   * @brief Helper method to create a mock callable, that uses
   * a given executor
   *
   */
  std::string addCallable(const std::string& parent_id,
      const BuildInfo& element_info, const ExecutorPtr& executor);

  std::unique_ptr<Device> result() final;

private:
  GroupMockPtr getParentGroup(const std::string& parent_id);

  std::string assignID(const std::string& parent_id);

  std::string makeElementMock(const std::string& parent_id,
      const ElementFunction& function, const BuildInfo& element_info);

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