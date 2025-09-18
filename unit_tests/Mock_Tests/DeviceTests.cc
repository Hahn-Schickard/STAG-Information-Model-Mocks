#include "DeviceMock.hpp"
#include "ElementMock.hpp"

#include <gtest/gtest.h>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

struct DeviceTests : public testing::Test {
  DeviceTests() {
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

    auto sub_readable = make_shared<NiceMock<ReadableMock>>(DataType::Boolean);
    sub_readable_id = sub_group->generateID();
    auto sub_readable_element =
        make_shared<NiceMock<ElementMock>>(sub_readable, sub_readable_id);
    sub_group->addElement(sub_readable_element);
  }

  string base_id = "based_id";
  DeviceMockPtr tested = make_shared<NiceMock<DeviceMock>>(
      base_id, FullMetaInfo{"test_device", "test device description"});
  string sub_group_id;
  string sub_readable_id;
  unordered_map<string, ElementMockPtr> built;
};

TEST_F(DeviceTests, returnsId) {
  EXPECT_CALL(*tested, id).Times(Exactly(1));

  EXPECT_EQ(tested->id(), base_id);
}

TEST_F(DeviceTests, returnsName) {
  EXPECT_CALL(*tested, name).Times(Exactly(1));

  EXPECT_EQ(tested->name(), "test_device");
}

TEST_F(DeviceTests, returnsDescription) {
  EXPECT_CALL(*tested, description).Times(Exactly(1));

  EXPECT_EQ(tested->description(), "test device description");
}

TEST_F(DeviceTests, addElementThrows) {
  EXPECT_THAT([&]() { tested->addElement(nullptr); },
      ThrowsMessage<invalid_argument>(HasSubstr("Given element is empty")));

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
            make_shared<ReadableMock>(DataType::Opaque), base_id + ":0"));
      },
      ThrowsMessage<logic_error>(HasSubstr(
          "Element with id " + base_id + ":0 is already in this group")));
}

TEST_F(DeviceTests, throwsElementNotFound) {
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
      "Element with reference id " + sub_readable_id + "0 was not found";
  EXPECT_THAT([&]() { tested->element(sub_readable_id + "0"); },
      ThrowsMessage<ElementNotFound>(HasSubstr(ex_msg3)));
}

TEST_F(DeviceTests, throwsIDPointsThisGroup) {
  EXPECT_THAT([&]() { tested->element(base_id); },
      ThrowsMessage<IDPointsThisGroup>(HasSubstr(
          "Reference ID " + base_id + " points to this group element")));
}

TEST_F(DeviceTests, isCorrectSize) { EXPECT_EQ(tested->size(), built.size()); }

TEST_F(DeviceTests, canGetElementById) {
  for (const auto& [id, element] : built) {
    EXPECT_NO_THROW({
      auto tested_element = tested->element(id);

      EXPECT_EQ(tested_element->id(), element->id());
      EXPECT_EQ(tested_element->name(), element->name());
      EXPECT_EQ(tested_element->description(), element->description());
      EXPECT_EQ(tested_element->type(), element->type());
    });
  }
}

TEST_F(DeviceTests, canVisitEach) {
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