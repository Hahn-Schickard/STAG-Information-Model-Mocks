#include "ObservableMock.hpp"
#include "TestResources.hpp"

#include <gtest/gtest.h>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

// NOLINTNEXTLINE(readability-identifier-naming)
MATCHER_P(ExceptionPointee, exception_type, "") {
  try {
    rethrow_exception(arg);
    return false;
  } catch (const exception& ex) {
    return typeid(ex) == typeid(exception_type) &&
        ex.what() == exception_type.what();
  }
}

struct ObservableTests : public TestWithParam<ReadableTestParam> {
  ObservableTests() {
    const auto& param = GetParam();
    expected_type = param.dataType();
    expected_variant = param.readResult();
    if (param.hasReadCallback()) {
      tested = make_shared<NiceMock<ObservableMock>>(
          expected_type, param.readCallback());
    } else {
      tested = make_shared<NiceMock<ObservableMock>>(expected_variant);
    }
    tested->enableSubscribeFaking(mock_enable_observation.AsStdFunction());
  }

  MockFunction<void(bool)> mock_enable_observation;
  DataType expected_type;
  DataVariant expected_variant;
  ObservableMockPtr tested;
};

TEST_P(ObservableTests, returnsDataType) {
  EXPECT_CALL(*tested, dataType).Times(Exactly(1));

  EXPECT_EQ(tested->dataType(), expected_type);
}

TEST_P(ObservableTests, canChangeDataType) {
  EXPECT_CALL(*tested, dataType).Times(Exactly(1));

  EXPECT_NO_THROW(tested->updateType(otherThan(expected_type)););

  EXPECT_NE(tested->dataType(), expected_type);
}

TEST_P(ObservableTests, canChangeReadValue) {
  EXPECT_CALL(*tested, read).Times(Exactly(2));

  EXPECT_EQ(tested->read(), expected_variant);

  EXPECT_NO_THROW(tested->updateValue(otherThan(expected_variant)););

  EXPECT_NE(tested->read(), expected_variant);
}

TEST_P(ObservableTests, canRead) {
  EXPECT_CALL(*tested, read).Times(Exactly(1));

  EXPECT_EQ(tested->read(), expected_variant);
}

TEST_P(ObservableTests, canReadTwice) {
  EXPECT_CALL(*tested, read).Times(Exactly(2));

  EXPECT_EQ(tested->read(), expected_variant);
  EXPECT_EQ(tested->read(), expected_variant);
}

TEST_P(ObservableTests, canUnsetCallback) {
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

TEST_P(ObservableTests, canUnsetReadCallback) {
  EXPECT_CALL(*tested, read).Times(Exactly(1));

  tested->updateReadCallback(nullptr);

  EXPECT_THROW(tested->read(), ReadCallbackUnavailable);
}

TEST_P(ObservableTests, throwsOnEmptyCallback) {
  MockFunction<void(const shared_ptr<DataVariant>&)> mock_observer_cb;
  MockFunction<void(const exception_ptr&)> mock_exception_handler;

  EXPECT_CALL(mock_enable_observation, Call(_)).Times(Exactly(0));
  EXPECT_CALL(*tested, subscribe).Times(Exactly(3));
  EXPECT_CALL(mock_observer_cb, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_exception_handler, Call(_)).Times(Exactly(0));

  auto throws_on_both_args = [this]() {
    auto connection = tested->subscribe(nullptr, nullptr);
  };
  EXPECT_THAT(throws_on_both_args,
      ThrowsMessage<invalid_argument>(
          HasSubstr("ObserveCallback can not be empty")));

  auto throws_on_first_arg = [this, &mock_exception_handler]() {
    auto connection =
        tested->subscribe(nullptr, mock_exception_handler.AsStdFunction());
  };
  EXPECT_THAT(throws_on_first_arg,
      ThrowsMessage<invalid_argument>(
          HasSubstr("ObserveCallback can not be empty")));

  auto throws_on_second_arg = [this, &mock_observer_cb]() {
    auto connection =
        tested->subscribe(mock_observer_cb.AsStdFunction(), nullptr);
  };
  EXPECT_THAT(throws_on_second_arg,
      ThrowsMessage<invalid_argument>(
          HasSubstr("ExceptionHandler can not be empty")));
}

TEST_P(ObservableTests, canSubAndUnsub) {
  MockFunction<void(const shared_ptr<DataVariant>&)> mock_observer_cb;
  MockFunction<void(const exception_ptr&)> mock_exception_handler;

  EXPECT_CALL(mock_enable_observation, Call(false)).Times(Exactly(1));
  EXPECT_CALL(mock_enable_observation, Call(true)).Times(Exactly(1));
  EXPECT_CALL(*tested, subscribe).Times(Exactly(1));
  EXPECT_CALL(mock_observer_cb, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_exception_handler, Call(_)).Times(Exactly(0));

  EXPECT_NO_THROW({
    auto connection = tested->subscribe(mock_observer_cb.AsStdFunction(),
        mock_exception_handler.AsStdFunction());

    connection.reset();

    // we must dispatch a notification for the mock to cleanup connections
    tested->notify(true);
  });
}

TEST_P(ObservableTests, canNotifyOne) {
  MockFunction<void(const shared_ptr<DataVariant>&)> mock_observer_cb;
  MockFunction<void(const exception_ptr&)> mock_exception_handler;

  EXPECT_CALL(mock_enable_observation, Call(true)).Times(Exactly(1));
  EXPECT_CALL(*tested, subscribe).Times(Exactly(1));
  EXPECT_CALL(mock_observer_cb, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_observer_cb, Call(Pointee(otherThan(expected_variant))))
      .Times(Exactly(1));
  EXPECT_CALL(mock_observer_cb, Call(Pointee(expected_variant)))
      .Times(Exactly(1));
  EXPECT_CALL(mock_exception_handler, Call(_)).Times(Exactly(0));

  EXPECT_NO_THROW({
    auto connection = tested->subscribe(mock_observer_cb.AsStdFunction(),
        mock_exception_handler.AsStdFunction());

    tested->notify(expected_variant);
    tested->notify(otherThan(expected_variant));
  });
}

TEST_P(ObservableTests, canNotifyTwo) {
  MockFunction<void(const shared_ptr<DataVariant>&)> mock_observer_1_cb;
  MockFunction<void(const shared_ptr<DataVariant>&)> mock_observer_2_cb;
  MockFunction<void(const exception_ptr&)> mock_exception_handler;

  EXPECT_CALL(mock_enable_observation, Call(true)).Times(Exactly(1));
  EXPECT_CALL(*tested, subscribe).Times(Exactly(2));
  EXPECT_CALL(mock_observer_1_cb, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_observer_1_cb, Call(Pointee(otherThan(expected_variant))))
      .Times(Exactly(1));
  EXPECT_CALL(mock_observer_1_cb, Call(Pointee(expected_variant)))
      .Times(Exactly(2));
  EXPECT_CALL(mock_observer_2_cb, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_observer_2_cb, Call(Pointee(otherThan(expected_variant))))
      .Times(Exactly(1));
  EXPECT_CALL(mock_observer_2_cb, Call(Pointee(expected_variant)))
      .Times(Exactly(1));
  EXPECT_CALL(mock_exception_handler, Call(_)).Times(Exactly(0));

  EXPECT_NO_THROW({
    auto connection_1 = tested->subscribe(mock_observer_1_cb.AsStdFunction(),
        mock_exception_handler.AsStdFunction());
    auto connection_2 = tested->subscribe(mock_observer_2_cb.AsStdFunction(),
        mock_exception_handler.AsStdFunction());

    tested->notify(expected_variant);
    tested->notify(otherThan(expected_variant));

    connection_2.reset();

    tested->notify(expected_variant);
  });
}

TEST_P(ObservableTests, canHandleSharedExceptions) {
  MockFunction<void(const shared_ptr<DataVariant>&)> mock_observer_cb;
  MockFunction<void(const exception_ptr&)> mock_exception_handler;
  runtime_error test_exception{"Test thrown exceptions in shared handler"};

  EXPECT_CALL(mock_enable_observation, Call(true)).Times(Exactly(1));
  EXPECT_CALL(*tested, subscribe).Times(Exactly(1));
  EXPECT_CALL(mock_observer_cb, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_observer_cb, Call(Pointee(otherThan(expected_variant))))
      .Times(Exactly(1));
  EXPECT_CALL(mock_observer_cb, Call(Pointee(expected_variant)))
      .Times(Exactly(1))
      .WillOnce(Throw(test_exception));
  EXPECT_CALL(mock_exception_handler, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_exception_handler, Call(ExceptionPointee(test_exception)))
      .Times(Exactly(1));

  EXPECT_NO_THROW({
    auto connection = tested->subscribe(mock_observer_cb.AsStdFunction(),
        mock_exception_handler.AsStdFunction());

    tested->notify(expected_variant);
    tested->notify(otherThan(expected_variant));
  });
}

TEST_P(ObservableTests, canHandleSeparateExceptions) {
  MockFunction<void(const shared_ptr<DataVariant>&)> mock_observer_1_cb;
  MockFunction<void(const shared_ptr<DataVariant>&)> mock_observer_2_cb;
  MockFunction<void(const exception_ptr&)> mock_exception_handler_1;
  MockFunction<void(const exception_ptr&)> mock_exception_handler_2;
  runtime_error test_exception{"Test thrown exceptions in separate handler"};

  EXPECT_CALL(mock_enable_observation, Call(true)).Times(Exactly(1));
  EXPECT_CALL(*tested, subscribe).Times(Exactly(2));
  EXPECT_CALL(mock_observer_1_cb, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_observer_1_cb, Call(Pointee(otherThan(expected_variant))))
      .Times(Exactly(1));
  EXPECT_CALL(mock_observer_1_cb, Call(Pointee(expected_variant)))
      .Times(Exactly(2));
  EXPECT_CALL(mock_observer_2_cb, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_observer_2_cb, Call(Pointee(otherThan(expected_variant))))
      .Times(Exactly(1));
  EXPECT_CALL(mock_observer_2_cb, Call(Pointee(expected_variant)))
      .Times(Exactly(1))
      .WillOnce(Throw(test_exception));
  EXPECT_CALL(mock_exception_handler_1, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_exception_handler_2, Call(_)).Times(Exactly(0));
  EXPECT_CALL(mock_exception_handler_2, Call(ExceptionPointee(test_exception)))
      .Times(Exactly(1));

  EXPECT_NO_THROW({
    auto connection_1 = tested->subscribe(mock_observer_1_cb.AsStdFunction(),
        mock_exception_handler_1.AsStdFunction());
    auto connection_2 = tested->subscribe(mock_observer_2_cb.AsStdFunction(),
        mock_exception_handler_2.AsStdFunction());

    tested->notify(expected_variant);
    tested->notify(otherThan(expected_variant));

    connection_2.reset();

    tested->notify(expected_variant);
  });
}

// NOLINTBEGIN(readability-magic-numbers)
INSTANTIATE_TEST_SUITE_P(ObservableTestsValues,
    ObservableTests,
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
    [](const TestParamInfo<ObservableTests::ParamType>& info) {
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