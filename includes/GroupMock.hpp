
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

  /**
   * @brief Generates a new ID string based on the mocked group ID
   * This method is intended to be used in tandem with addElement() method when
   * creating new Element instances.
   *
   * Used by MockBuilder class
   *
   * @attention We strongly recommend using the MockBuilder to build GroupMock
   * instances instead of manually building the mocks yourself
   *
   * @return std::string
   */
  std::string generateID();

  /**
   * @brief Checks if a given element instance is part of this Device and adds
   * it to the device, if that is the case
   *
   * @throws std::invalid_argument - if
   * - given element is null
   * - given element id is the same as this group id
   * - given element id is not part of this group
   * - given element id points to a parent element that is not a group
   * @throws std::logic_error - if given element is already in this group
   *
   * @param element
   */
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