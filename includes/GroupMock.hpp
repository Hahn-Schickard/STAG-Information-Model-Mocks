
#ifndef __STAG_INFORMATION_MODEL_MOCKS_GROUP_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_GROUP_MOCK_HPP
#include <Information_Model/Group.hpp>
#include <gmock/gmock.h>

namespace Information_Model::testing {

struct GroupMock : public Group {
  explicit GroupMock(const std::string& id);

  ~GroupMock() override = default;

  MOCK_METHOD(size_t, size, (), (const final));
  MOCK_METHOD(
      (std::unordered_map<std::string, ElementPtr>), asMap, (), (const final));
  MOCK_METHOD(std::vector<ElementPtr>, asVector, (), (const final));
  MOCK_METHOD(ElementPtr, element, (const std::string&), (const final));
  MOCK_METHOD(void, visit, (const Group::Visitor&), (const final));

  std::string generateID();
  void addElement(const ElementPtr& element);

private:
  ElementPtr getElement(const std::string& ref_id);

  std::unordered_map<std::string, ElementPtr> elements_;
  std::unordered_map<std::string, GroupPtr> subgroups_;
  size_t next_id_ = 0;
  std::string id_;
};

using GroupMockPtr = std::shared_ptr<GroupMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_GROUP_MOCK_HPP