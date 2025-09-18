
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

  std::string generateID();
  void addElement(const ElementPtr& element);

private:
  GroupMockPtr group_;
};
using DeviceMockPtr = std::shared_ptr<DeviceMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_DEVICE_MOCK_HPP