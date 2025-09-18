#include "ElementMock.hpp"

#include <gtest/gtest.h>
#include <optional>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

TEST(ElementUtilTests, toStringThrows) {
  string result{};
  // we deliberately want to test how out of range casts are handled
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
  EXPECT_THAT([&result]() { result = toString(static_cast<ElementType>(230)); },
      ThrowsMessage<logic_error>(
          HasSubstr("Could not decode ElementType enum value")));

  EXPECT_TRUE(result.empty());
}

struct ElementTestParam {
  string id;
  string name;
  string description;
  ElementType element_type;
  DataType data_type;
  ElementFunction function;

  // NOLINTNEXTLINE(readability-identifier-naming)
  friend void PrintTo(const ElementTestParam& param, std::ostream* os) {
    *os << "(id: " << param.id << ", name:" << param.name
        << ", description:" << param.description
        << ", element_type:" << toString(param.element_type)
        << ", data_type:" << toString(param.data_type) << ")";
  }
};

struct ElementTests : public TestWithParam<ElementTestParam> {
  ElementTests() {
    expected = GetParam();
    tested = make_shared<ElementMock>(expected.function,
        expected.id,
        FullMetaInfo{expected.name, expected.description});
  }

  ElementTestParam expected;
  ElementMockPtr tested;
};

TEST_P(ElementTests, canGetType) {
  EXPECT_CALL(*tested, type).Times(Exactly(1));

  EXPECT_EQ(tested->type(), expected.element_type);
}

TEST_P(ElementTests, canGetFunction) {
  EXPECT_CALL(*tested, function).Times(Exactly(1));

  EXPECT_EQ(tested->function(), expected.function);
}

INSTANTIATE_TEST_SUITE_P(ElementTestsValues,
    ElementTests,
    Values( // clang-format off
        ElementTestParam{
            "readable_bool",
            "readable_element",
            "description of a readable element",
            ElementType::Readable,
            DataType::Boolean,
            make_shared<ReadableMock>(DataType::Boolean)
        },
        ElementTestParam{
            "writable_string",
            "writable_element",
            "description of a writable element",
            ElementType::Writable,
            DataType::String,
            make_shared<WritableMock>(DataType::String)
        },
        ElementTestParam{
            "observable_double",
            "observable_element",
            "description of a observable element",
            ElementType::Observable,
            DataType::Double,
            make_shared<ObservableMock>(DataType::Double)
        },
        ElementTestParam{
            "callable_opaque",
            "callable_element",
            "description of a callable element",
            ElementType::Callable,
            DataType::Opaque,
            make_shared<CallableMock>(DataType::Opaque)
        },
        ElementTestParam{
            "group",
            "group_element",
            "description of a group element",
            ElementType::Group,
            DataType::None,
            make_shared<GroupMock>("group:0")
        }
    ), // clang-format on
    [](const TestParamInfo<ElementTests::ParamType>& info) {
      return info.param.id;
    });

} // namespace Information_Model::testing