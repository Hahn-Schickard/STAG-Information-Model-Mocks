#include "ReadableMock.hpp"
#include "TestResources.hpp"

#include <gtest/gtest.h>
#include <optional>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

struct ReadableTests : public TestWithParam<ReadableTestParam> {
  ReadableTests() {
    const auto& param = GetParam();
    expected_type = param.dataType();
    expected_variant = param.readResult();
    if (param.hasReadCallback()) {
      tested = make_shared<ReadableMock>(expected_type, param.readCallback());
    } else {
      tested = make_shared<ReadableMock>(expected_variant);
    }
  }

  DataType expected_type;
  DataVariant expected_variant;
  ReadableMockPtr tested;
};

TEST_P(ReadableTests, returnsDataType) {
  EXPECT_CALL(*tested, dataType).Times(Exactly(1));

  EXPECT_EQ(tested->dataType(), expected_type);
}

TEST_P(ReadableTests, canChangeDataType) {
  EXPECT_CALL(*tested, dataType).Times(Exactly(1));

  EXPECT_NO_THROW(tested->updateType(otherThan(expected_type)););

  EXPECT_NE(tested->dataType(), toDataType(expected_variant));
}

TEST_P(ReadableTests, canChangeReadValue) {
  EXPECT_CALL(*tested, read).Times(Exactly(2));

  EXPECT_EQ(tested->read(), expected_variant);

  EXPECT_NO_THROW(tested->updateValue(otherThan(expected_variant)););

  EXPECT_NE(tested->read(), expected_variant);
}

TEST_P(ReadableTests, canRead) {
  EXPECT_CALL(*tested, read).Times(Exactly(1));

  EXPECT_EQ(tested->read(), expected_variant);
}

TEST_P(ReadableTests, canReadTwice) {
  EXPECT_CALL(*tested, read).Times(Exactly(2));

  EXPECT_EQ(tested->read(), expected_variant);
  EXPECT_EQ(tested->read(), expected_variant);
}

TEST_P(ReadableTests, canChangeCallback) {
  MockFunction<DataVariant()> mock_readable;
  EXPECT_CALL(*tested, read).Times(Exactly(2));
  EXPECT_CALL(mock_readable, Call())
      .Times(Exactly(2))
      .WillRepeatedly(Return(otherThan(expected_variant)));

  tested->updateReadCallback(mock_readable.AsStdFunction());

  auto read_value = tested->read();
  EXPECT_NE(read_value, expected_variant);
  EXPECT_EQ(read_value, otherThan(expected_variant));
  EXPECT_EQ(tested->read(), otherThan(expected_variant));
}

TEST_P(ReadableTests, canUnsetCallback) {
  EXPECT_CALL(*tested, read).Times(Exactly(1));

  tested->updateReadCallback(nullptr);

  EXPECT_THROW(tested->read(), ReadCallbackUnavailable);
}

// NOLINTBEGIN(readability-magic-numbers)
INSTANTIATE_TEST_SUITE_P(ReadableTestsValues,
    ReadableTests,
    Values( // clang-format off
        ReadableTestParam{DataVariant(true)},
        ReadableTestParam{DataVariant(false)},
        ReadableTestParam{DataVariant(intmax_t{15})},
        ReadableTestParam{DataVariant(intmax_t{-20})},
        ReadableTestParam{DataVariant(uintmax_t{69})},
        ReadableTestParam{DataVariant(3.14)},
        ReadableTestParam{DataVariant(-45.33333333)},
        ReadableTestParam{DataVariant(Timestamp{2025, 9, 11, 10, 01, 24, 32})},
        ReadableTestParam{DataVariant(vector<uint8_t>{})},
        ReadableTestParam{DataVariant(vector<uint8_t>{0x11,0x22,0x31})},
        ReadableTestParam{DataVariant(string())},
        ReadableTestParam{DataVariant(string("Hello World"))},
        ReadableTestParam{DataType::Boolean,
            [](){ return DataVariant(true);}},
        ReadableTestParam{DataType::Boolean,
            [](){ return DataVariant(false);}},
        ReadableTestParam{DataType::Integer,
            [](){ return DataVariant(intmax_t{15});}},
        ReadableTestParam{DataType::Integer,
            [](){ return DataVariant(intmax_t{-20});}},
        ReadableTestParam{DataType::Unsigned_Integer,
            [](){ return DataVariant(uintmax_t{69});}},
        ReadableTestParam{DataType::Double,
            [](){ return DataVariant(3.14);}},
        ReadableTestParam{DataType::Double,
            [](){ return DataVariant(-45.33333333);}},
        ReadableTestParam{DataType::Timestamp,
            [](){ return DataVariant(Timestamp{2025, 9, 11, 10, 01, 24, 32});}},
        ReadableTestParam{DataType::Opaque,
            [](){ return DataVariant(vector<uint8_t>{});}},
        ReadableTestParam{DataType::Opaque,
            [](){ return DataVariant(vector<uint8_t>{0x11,0x22,0x31});}},
        ReadableTestParam{DataType::String,
            [](){ return DataVariant(string());}},
        ReadableTestParam{DataType::String,
            [](){ return DataVariant(string("Hello World"));}}
    ), // clang-format on
    [](const TestParamInfo<ReadableTests::ParamType>& info) {
      string name;
      if (info.param.hasReadCallback()) {
        name += "callback";
      } else {
        name += "value";
      }
      return name + toSanitizedString(info.param.readResult());
    });
// NOLINTEND(readability-magic-numbers)
} // namespace Information_Model::testing