
#ifndef __STAG_INFORMATION_MODEL_MOCKS_DEVICE_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_DEVICE_MOCK_HPP
#include "GroupMock.hpp"
#include "MetaInfoMock.hpp"

#include <Information_Model/Device.hpp>
#include <gmock/gmock.h>

namespace Information_Model::testing {

struct DeviceMock : virtual public Device, public MetaInfoMock {
  explicit DeviceMock(const std::string& base_id,
      const std::optional<FullMetaInfo>& meta = std::nullopt);

  ~DeviceMock() override = default;

  MOCK_METHOD(GroupPtr, group, (), (const final));
  MOCK_METHOD(size_t, size, (), (const final));
  MOCK_METHOD(ElementPtr, element, (const std::string&), (const final));
  MOCK_METHOD(void, visit, (const Group::Visitor&), (const final));

  /**
   * @brief Generates a new ID string based on the mocked device ID
   * This method is intended to be used in tandem with addElement() method when
   * creating new root Element instances.
   *
   * Used by MockBuilder class
   *
   * @attention We strongly recommend using the MockBuilder to build DeviceMock
   * instances instead of manually building the mocks yourself
   *
   * @return std::string
   */
  std::string generateID();

  /**
   * @brief Checks if a given element instance is part of this Device and adds
   * it to the device, if that is the case
   *
   * @throws std::invalid_argument - same as @ref GroupMock::addElement()
   * @throws std::logic_error - same as @ref GroupMock::addElement()
   *
   * @param element
   */
  void addElement(const ElementPtr& element);

private:
  GroupMockPtr group_;
};
using DeviceMockPtr = std::shared_ptr<DeviceMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_DEVICE_MOCK_HPP