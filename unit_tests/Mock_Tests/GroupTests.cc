#include "ElementMock.hpp"
#include "GroupMock.hpp"

#include <gtest/gtest.h>
#include <unordered_map>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

struct GroupTests : public testing::Test {
  GroupTests() {
    auto readable = make_shared<NiceMock<ReadableMock>>(DataType::Boolean);
    auto readable_id = tested->generateID();
    auto readable_element =
        make_shared<NiceMock<ElementMock>>(readable, readable_id);
    tested->addElement(readable_element);
    built.try_emplace(readable_id, readable_element);

    auto writable = make_shared<NiceMock<WritableMock>>(DataType::Double);
    auto writable_id = tested->generateID();
    auto writable_element =
        make_shared<NiceMock<ElementMock>>(writable, writable_id);
    tested->addElement(writable_element);
    built.try_emplace(writable_id, writable_element);

    auto observable = make_shared<NiceMock<ObservableMock>>(DataType::String);
    auto observable_id = tested->generateID();
    auto observable_element =
        make_shared<NiceMock<ElementMock>>(observable, observable_id);
    tested->addElement(observable_element);
    built.try_emplace(observable_id, observable_element);

    auto callable =
        make_shared<NiceMock<CallableMock>>(DataType::Unsigned_Integer);
    auto callable_id = tested->generateID();
    auto callable_element =
        make_shared<NiceMock<ElementMock>>(callable, callable_id);
    tested->addElement(callable_element);
    built.try_emplace(callable_id, callable_element);

    sub_group_id = tested->generateID();
    auto sub_group = make_shared<NiceMock<GroupMock>>(sub_group_id);
    auto sub_group_element =
        make_shared<NiceMock<ElementMock>>(sub_group, sub_group_id);
    tested->addElement(sub_group_element);
    built.try_emplace(sub_group_id, sub_group_element);

    sub_element =
        make_shared<NiceMock<ElementMock>>(readable, sub_group->generateID());
    sub_group->addElement(sub_element);

    auto sub_sub_group_id = sub_group->generateID();
    sub_sub_group = make_shared<NiceMock<GroupMock>>(sub_sub_group_id);
    sub_group->addElement(
        make_shared<NiceMock<ElementMock>>(sub_sub_group, sub_sub_group_id));
    sub_sub_element = make_shared<NiceMock<ElementMock>>(
        readable, sub_sub_group->generateID());
    sub_sub_group->addElement(sub_sub_element);
  }

  string base_id = "based_id:0";
  GroupMockPtr tested = make_shared<NiceMock<GroupMock>>(base_id);
  string sub_group_id;
  ElementPtr sub_element;
  GroupMockPtr sub_sub_group;
  ElementPtr sub_sub_element;
  unordered_map<string, ElementMockPtr> built;
};

TEST_F(GroupTests, addElementThrows) {
  EXPECT_THAT([&]() { tested->addElement(nullptr); },
      ThrowsMessage<invalid_argument>(HasSubstr("Given element is empty")));

  EXPECT_THAT(
      [&]() {
        tested->addElement(make_shared<NiceMock<ElementMock>>(
            make_shared<ReadableMock>(DataType::Opaque), "based_id:1"));
      },
      ThrowsMessage<invalid_argument>(
          HasSubstr("Given element is not part of this group")));

  EXPECT_THAT(
      [&]() {
        tested->addElement(make_shared<NiceMock<ElementMock>>(
            make_shared<ReadableMock>(DataType::Opaque), "another_id:0"));
      },
      ThrowsMessage<invalid_argument>(
          HasSubstr("Given element is not part of this group")));

  EXPECT_THAT(
      [&]() {
        tested->addElement(make_shared<NiceMock<ElementMock>>(
            make_shared<ReadableMock>(DataType::Opaque), base_id));
      },
      ThrowsMessage<invalid_argument>(
          HasSubstr("Given element has the same ID as this group")));

  EXPECT_THAT(
      [&]() {
        tested->addElement(make_shared<NiceMock<ElementMock>>(
            make_shared<ReadableMock>(DataType::Opaque), base_id + ".0"));
      },
      ThrowsMessage<logic_error>(HasSubstr(
          "Element with id " + base_id + ".0 is already in this group")));
}

TEST_F(GroupTests, throwsElementNotFound) {
  EXPECT_THAT([&]() { tested->element("bad_ref"); },
      ThrowsMessage<ElementNotFound>(
          HasSubstr("Element with reference id bad_ref was not found")));

  string ex_msg1 =
      "Element with reference id " + sub_group_id + ".2 was not found";
  EXPECT_THAT([&]() { tested->element(sub_group_id + ".2"); },
      ThrowsMessage<ElementNotFound>(HasSubstr(ex_msg1)));

  string ex_msg2 =
      "Element with reference id " + sub_group_id + "0 was not found";
  EXPECT_THAT([&]() { tested->element(sub_group_id + "0"); },
      ThrowsMessage<ElementNotFound>(HasSubstr(ex_msg2)));

  string ex_msg3 =
      "Element with reference id " + sub_sub_element->id() + "0 was not found";
  EXPECT_THAT([&]() { tested->element(sub_sub_element->id() + "0"); },
      ThrowsMessage<ElementNotFound>(HasSubstr(ex_msg3)));
}

TEST_F(GroupTests, throwsIDPointsThisGroup) {
  EXPECT_THAT([&]() { tested->element(base_id); },
      ThrowsMessage<IDPointsThisGroup>(HasSubstr(
          "Reference ID " + base_id + " points to this group element")));
}

TEST_F(GroupTests, isCorrectSize) { EXPECT_EQ(tested->size(), built.size()); }

TEST_F(GroupTests, canGetElementById) {
  for (const auto& [id, element] : built) {
    auto tested_element = tested->element(id);

    EXPECT_EQ(tested_element->id(), element->id());
    EXPECT_EQ(tested_element->name(), element->name());
    EXPECT_EQ(tested_element->description(), element->description());
    EXPECT_EQ(tested_element->type(), element->type());
  }

  auto tested_sub_element = tested->element(sub_element->id());
  EXPECT_EQ(tested_sub_element->id(), sub_element->id());
  EXPECT_EQ(tested_sub_element->name(), sub_element->name());
  EXPECT_EQ(tested_sub_element->description(), sub_element->description());
  EXPECT_EQ(tested_sub_element->type(), sub_element->type());

  auto tested_sub_sub_element = tested->element(sub_sub_element->id());
  EXPECT_EQ(tested_sub_sub_element->id(), sub_sub_element->id());
  EXPECT_EQ(tested_sub_sub_element->name(), sub_sub_element->name());
  EXPECT_EQ(
      tested_sub_sub_element->description(), sub_sub_element->description());
  EXPECT_EQ(tested_sub_sub_element->type(), sub_sub_element->type());
}

TEST_F(GroupTests, canGetAsMap) {
  unordered_map<string, ElementPtr> built_as_base;
  for (const auto& [id, element] : built) {
    auto correct_id = id.substr(base_id.size());
    if (correct_id.front() == '.') {
      correct_id.erase(0, 1);
    }
    if (correct_id.back() == '.') {
      correct_id.pop_back();
    }
    built_as_base.try_emplace(correct_id, element);
  }

  EXPECT_THAT(tested->asMap(), ContainerEq(built_as_base));
}

TEST_F(GroupTests, canGetAsVector) {
  vector<ElementPtr> built_as_vector;
  built_as_vector.reserve(built.size());
  for (const auto& [_, element] : built) {
    built_as_vector.push_back(element);
  }
  auto predicate = [](const ElementPtr& lhs, const ElementPtr& rhs) {
    return lhs->id() < rhs->id();
  };
  sort(built_as_vector.begin(), built_as_vector.end(), predicate);

  auto tested_vector = tested->asVector();
  EXPECT_THAT(tested_vector, ContainerEq(built_as_vector));
}

TEST_F(GroupTests, canVisitEach) {
  auto visitor = [&](const ElementPtr& tested_element) {
    auto it = built.find(tested_element->id());
    if (it != built.end()) {
      auto element = it->second;
      EXPECT_EQ(tested_element->id(), element->id());
      EXPECT_EQ(tested_element->name(), element->name());
      EXPECT_EQ(tested_element->description(), element->description());
      EXPECT_EQ(tested_element->type(), element->type());
    } else {
      FAIL() << "Element " << tested_element->id()
             << " was not built but exists in group" << endl;
    }
  };

  EXPECT_NO_THROW(tested->visit(visitor));
}

} // namespace Information_Model::testing