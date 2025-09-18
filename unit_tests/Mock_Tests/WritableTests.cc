#include "TestResources.hpp"
#include "WritableMock.hpp"

#include <gtest/gtest.h>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

struct WritableTestParam : public ReadableTestParam {
  using ReadCallback = ReadableTestParam::ReadCallback;
  using WriteCallback = std::function<void(const DataVariant&)>;

  explicit WritableTestParam(DataType type)
      : ReadableTestParam(type), write_only_(true) {}

  explicit WritableTestParam(const DataVariant& value)
      : ReadableTestParam(value) {}

  bool isWriteOnly() const { return write_only_; }

  // NOLINTNEXTLINE(readability-identifier-naming)
  friend void PrintTo(const WritableTestParam& param, std::ostream* os) {
    *os << "(write_only: " << param.write_only_
        << (param.write_only_ ? "" : ", value: " + toString(param.value_))
        << ", type: " << toString(param.type_) << ")";
  }

protected:
  bool write_only_ = false;
};

struct WritableTests : public TestWithParam<WritableTestParam> {
  WritableTests() {
    const auto& param = GetParam();
    expected_type = param.dataType();
    expected_variant = param.readResult();
    if (param.hasReadCallback()) {
      tested = make_shared<NiceMock<WritableMock>>(
          expected_type, param.readCallback(), mock_writable.AsStdFunction());
    } else {
      tested = make_shared<NiceMock<WritableMock>>(
          expected_type, mock_writable.AsStdFunction());
    }
  }

  MockFunction<void(const DataVariant&)> mock_writable;
  DataType expected_type;
  DataVariant expected_variant;
  WritableMockPtr tested;
};

TEST_P(WritableTests, returnsDataType) {
  EXPECT_CALL(*tested, dataType).Times(Exactly(1));

  EXPECT_EQ(tested->dataType(), expected_type);
}

TEST_P(WritableTests, canChangeDataType) {
  EXPECT_CALL(*tested, dataType).Times(Exactly(1));

  EXPECT_NO_THROW(tested->updateType(otherThan(expected_type)););

  EXPECT_NE(tested->dataType(), expected_type);
}

TEST_P(WritableTests, canChangeReadValue) {
  if (!tested->isWriteOnly()) {
    EXPECT_CALL(*tested, read).Times(Exactly(2));

    EXPECT_EQ(tested->read(), expected_variant);

    EXPECT_NO_THROW(tested->updateValue(otherThan(expected_variant)););

    EXPECT_NE(tested->read(), expected_variant);
  }
}

TEST_P(WritableTests, canRead) {
  EXPECT_CALL(*tested, read).Times(Exactly(1));

  if (tested->isWriteOnly()) {
    EXPECT_THROW(tested->read(), NonReadable);
  } else {
    EXPECT_EQ(tested->read(), expected_variant);
  }
}

TEST_P(WritableTests, canReadTwice) {
  EXPECT_CALL(*tested, read).Times(Exactly(2));

  if (tested->isWriteOnly()) {
    EXPECT_THROW(tested->read(), NonReadable);
    EXPECT_THROW(tested->read(), NonReadable);
  } else {
    EXPECT_EQ(tested->read(), expected_variant);
    EXPECT_EQ(tested->read(), expected_variant);
  }
}

TEST_P(WritableTests, canChangeReadCallback) {
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

TEST_P(WritableTests, canWrite) {
  EXPECT_CALL(*tested, write).Times(Exactly(1));

  EXPECT_CALL(mock_writable, Call(expected_variant)).Times(Exactly(1));

  EXPECT_NO_THROW(tested->write(expected_variant));
}

TEST_P(WritableTests, canWriteTwice) {
  EXPECT_CALL(*tested, write).Times(Exactly(2));

  EXPECT_CALL(mock_writable, Call(otherThan(expected_variant)))
      .Times(Exactly(1));
  EXPECT_CALL(mock_writable, Call(expected_variant)).Times(Exactly(1));

  EXPECT_NO_THROW(tested->write(expected_variant));
  EXPECT_NO_THROW(tested->write(otherThan(expected_variant)));
}

TEST_P(WritableTests, canReadWritten) {
  EXPECT_CALL(*tested, write).Times(Exactly(1));
  EXPECT_CALL(*tested, read).Times(Exactly(2));

  EXPECT_CALL(mock_writable, Call(otherThan(expected_variant)))
      .Times(Exactly(1));

  if (tested->isWriteOnly()) {
    EXPECT_THROW(tested->read(), NonReadable);
    EXPECT_NO_THROW(tested->write(otherThan(expected_variant)));
    EXPECT_THROW(tested->read(), NonReadable);
  } else {
    EXPECT_EQ(tested->read(), expected_variant);
    EXPECT_NO_THROW(tested->write(otherThan(expected_variant)));
    EXPECT_EQ(tested->read(), otherThan(expected_variant));
  }
}

TEST_P(WritableTests, canChangeWriteCallback) {
  MockFunction<void(const DataVariant&)> other_mock_writable;

  EXPECT_CALL(mock_writable, Call(_)).Times(Exactly(0));
  EXPECT_CALL(other_mock_writable, Call(otherThan(expected_variant)))
      .Times(Exactly(1));
  EXPECT_CALL(other_mock_writable, Call(expected_variant)).Times(Exactly(1));

  tested->updateWriteCallback(other_mock_writable.AsStdFunction());

  EXPECT_NO_THROW(tested->write(expected_variant));
  EXPECT_NO_THROW(tested->write(otherThan(expected_variant)));
}

TEST_P(WritableTests, canUnsetCallbacks) {
  EXPECT_CALL(*tested, read).Times(Exactly(1));
  EXPECT_CALL(*tested, write).Times(Exactly(1));

  tested->updateCallbacks(nullptr, nullptr);

  EXPECT_THROW(tested->read(), NonReadable);
  EXPECT_THROW(tested->write(true), WriteCallbackUnavailable);
}

// NOLINTBEGIN(readability-magic-numbers)
INSTANTIATE_TEST_SUITE_P(WritableTestsValues,
    WritableTests,
    Values( // clang-format off
        WritableTestParam{DataType::Boolean},
        WritableTestParam{DataType::Integer},
        WritableTestParam{DataType::Unsigned_Integer},
        WritableTestParam{DataType::Double},
        WritableTestParam{DataType::Timestamp},
        WritableTestParam{DataType::Opaque},
        WritableTestParam{DataType::String},
        WritableTestParam{DataVariant(true)},
        WritableTestParam{DataVariant(intmax_t{15})},
        WritableTestParam{DataVariant(intmax_t{-20})},
        WritableTestParam{DataVariant(uintmax_t{69})},
        WritableTestParam{DataVariant(3.14)},
        WritableTestParam{DataVariant(-45.33333333)},
        WritableTestParam{DataVariant(Timestamp{2025, 9, 11, 10, 01, 24, 32})},
        WritableTestParam{DataVariant(vector<uint8_t>{0x11,0x22,0x31})},
        WritableTestParam{DataVariant(string("Hello World"))}
    ), // clang-format on
    [](const TestParamInfo<WritableTests::ParamType>& info) {
      string name;
      if (info.param.isWriteOnly()) {
        name += "writes";
      } else {
        name += "reads";
      }
      return name + toSanitizedString(info.param.readResult());
    });
// NOLINTEND(readability-magic-numbers)
} // namespace Information_Model::testing