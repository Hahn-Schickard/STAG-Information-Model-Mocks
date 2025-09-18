#include "CallableMock.hpp"
#include "TestResources.hpp"

#include <Variant_Visitor/Visitor.hpp>
#include <gtest/gtest.h>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

string toString(const Executor::Response& response) {
  return Variant_Visitor::match(
      response,
      [](const DataVariant& value) { return toString(value); },
      [](const exception_ptr& exception) -> string {
        if (!exception) {
          return "No exception value";
        }
        try {
          rethrow_exception(exception);
        } catch (const std::exception& ex) {
          return ex.what();
        }
      });
}

struct CallableTestParam {
  string test_name;
  DataType result_type = DataType::None;
  ParameterTypes supported_params;
  Executor::Response default_response =
      make_exception_ptr(logic_error("Response should not return values"));
  Parameters parameters;

  // NOLINTNEXTLINE(readability-identifier-naming)
  friend void PrintTo(const CallableTestParam& param, std::ostream* os) {
    *os << "(result_type: " << toString(param.result_type)
        << ", supported_params: " << toString(param.supported_params)
        << ", default_response: " << toString(param.default_response)
        << ", parameters: " << toString(param.parameters) << ")";
  }
};

struct CallableTests : public TestWithParam<CallableTestParam> {
  CallableTests() : expected(GetParam()) {
    tested = make_shared<CallableMock>(expected.result_type,
        expected.supported_params,
        expected.default_response);
  }

  CallableTestParam expected;
  CallableMockPtr tested;
};

TEST_P(CallableTests, returnsResultType) {
  EXPECT_CALL(*tested, resultType).Times(Exactly(1));

  EXPECT_EQ(tested->resultType(), expected.result_type);
}

TEST_P(CallableTests, returnsParameterTypes) {
  EXPECT_CALL(*tested, parameterTypes).Times(Exactly(1));

  EXPECT_THAT(tested->parameterTypes(), ContainerEq(expected.supported_params));
}

TEST_P(CallableTests, canExecute) {
  EXPECT_CALL(*tested, execute).Times(Exactly(1));

  EXPECT_NO_THROW(tested->execute(expected.parameters));
}

TEST_P(CallableTests, canCall) {
  EXPECT_CALL(*tested, call(expected.parameters, 200)).Times(Exactly(1));

  if (expected.result_type != DataType::None) {
    auto executor = tested->getExecutor();
    executor->start();
    EXPECT_NO_THROW(tested->call(expected.parameters, 200));
    executor->stop();
  } else {
    EXPECT_THROW(
        tested->call(expected.parameters, 200), ResultReturningNotSupported);
  }
}

TEST_P(CallableTests, canTimeoutCall) {
  EXPECT_CALL(*tested, call(expected.parameters, 1)).Times(Exactly(1));

  if (expected.result_type != DataType::None) {
    EXPECT_THROW(tested->call(expected.parameters, 1), CallTimedout);
  } else {
    EXPECT_THROW(
        tested->call(expected.parameters, 1), ResultReturningNotSupported);
  }
}

TEST_P(CallableTests, canAsyncCall) {
  EXPECT_CALL(*tested, asyncCall(expected.parameters)).Times(Exactly(1));

  if (expected.result_type != DataType::None) {
    EXPECT_NO_THROW({
      auto result = tested->asyncCall(expected.parameters);

      auto executor = tested->getExecutor();
      executor->start();
      EXPECT_EQ(result.get(), get<DataVariant>(expected.default_response));
      executor->stop();
    });
  } else {
    EXPECT_THROW(
        { auto result = tested->asyncCall(expected.parameters); },
        ResultReturningNotSupported);
  }
}

TEST_P(CallableTests, canCancelAsyncCall) {
  EXPECT_CALL(*tested, asyncCall(expected.parameters)).Times(Exactly(1));

  if (expected.result_type != DataType::None) {
    EXPECT_CALL(*tested, cancelAsyncCall(_)).Times(Exactly(1));

    EXPECT_NO_THROW({
      auto result = tested->asyncCall(expected.parameters);

      tested->cancelAsyncCall(result.id());
      EXPECT_THROW(result.get(), CallCanceled);
    });
  } else {
    EXPECT_THROW(
        { auto result = tested->asyncCall(expected.parameters); },
        ResultReturningNotSupported);
  }
}

TEST_P(CallableTests, resultOutlivesAsyncCall) {
  if (expected.result_type != DataType::None) {
    EXPECT_CALL(*tested, asyncCall(expected.parameters)).Times(Exactly(2));

    auto result1 = tested->asyncCall(expected.parameters);
    {
      auto executor = tested->getExecutor();
      executor->respondOnce();
    }
    auto result2 = tested->asyncCall(expected.parameters);
    tested.reset();
    EXPECT_EQ(result1.get(), get<DataVariant>(expected.default_response));
    EXPECT_THROW(result2.get(), CallCanceled);
  } else {
    EXPECT_CALL(*tested, asyncCall(expected.parameters)).Times(Exactly(1));

    EXPECT_THROW(
        { auto result = tested->asyncCall(expected.parameters); },
        ResultReturningNotSupported);
  }
}

TEST_P(CallableTests, canUnsetExecutor) {
  EXPECT_CALL(*tested, execute(expected.parameters)).Times(Exactly(1));
  EXPECT_CALL(*tested, call(100)).Times(Exactly(1));
  EXPECT_CALL(*tested, call(expected.parameters, 100)).Times(Exactly(1));
  EXPECT_CALL(*tested, asyncCall(expected.parameters)).Times(Exactly(1));
  EXPECT_CALL(*tested, cancelAsyncCall(25)).Times(Exactly(1));

  tested->changeExecutor(nullptr);

  EXPECT_THAT([&]() { tested->execute(expected.parameters); },
      ThrowsMessage<ExecutorNotAvailable>(
          HasSubstr("Executor callback is no longer available")));

  EXPECT_THAT([&]() { auto result = tested->call(100); },
      ThrowsMessage<ExecutorNotAvailable>(
          HasSubstr("Executor callback is no longer available")));

  EXPECT_THAT([&]() { auto result = tested->call(expected.parameters, 100); },
      ThrowsMessage<ExecutorNotAvailable>(
          HasSubstr("Executor callback is no longer available")));

  EXPECT_THAT([&]() { auto result = tested->asyncCall(expected.parameters); },
      ThrowsMessage<ExecutorNotAvailable>(
          HasSubstr("Executor callback is no longer available")));

  EXPECT_THAT([&]() { tested->cancelAsyncCall(25); },
      ThrowsMessage<ExecutorNotAvailable>(
          HasSubstr("Executor callback is no longer available")));
}

TEST_P(CallableTests, executorThrowsCallerNotFound) {
  auto executor = tested->getExecutor();

  EXPECT_THAT([&]() { executor->respond(0, true); },
      ThrowsMessage<CallerNotFound>(HasSubstr(
          "No caller with id: 0 for Callable ExternalExecutor call exists")));

  EXPECT_THAT(
      [&]() {
        executor->respond(1, make_exception_ptr(runtime_error("Not thrown")));
      },
      ThrowsMessage<CallerNotFound>(HasSubstr(
          "No caller with id: 1 for Callable ExternalExecutor call exists")));
}

TEST_P(CallableTests, executorThrowsOnBadQueueResponse) {
  auto executor = tested->getExecutor();

  if (expected.result_type == DataType::None) {
    EXPECT_THAT([&]() { executor->queueResponse(true); },
        ThrowsMessage<invalid_argument>(
            HasSubstr("Can not set DataVariant response for executor. "
                      "Executor does not support returning values")));

    EXPECT_THAT([&]() { executor->queueResponse(1, true); },
        ThrowsMessage<invalid_argument>(
            HasSubstr("Can not set DataVariant response for executor. "
                      "Executor does not support returning values")));
  } else {
    EXPECT_THAT(
        [&]() {
          executor->queueResponse(
              otherThan(get<DataVariant>(expected.default_response)));
        },
        ThrowsMessage<invalid_argument>(
            HasSubstr("Executor is suppose to return " +
                toString(expected.result_type) + " data values, not " +
                toString(toDataType(
                    otherThan(get<DataVariant>(expected.default_response)))))));

    EXPECT_THAT(
        [&]() {
          executor->queueResponse(
              1, otherThan(get<DataVariant>(expected.default_response)));
        },
        ThrowsMessage<invalid_argument>(
            HasSubstr("Executor is suppose to return " +
                toString(expected.result_type) + " data values, not " +
                toString(toDataType(
                    otherThan(get<DataVariant>(expected.default_response)))))));
  }
}

// NOLINTBEGIN(readability-magic-numbers)
INSTANTIATE_TEST_SUITE_P(CallableTestsValues,
    CallableTests,
    Values( // clang-format off
        CallableTestParam{
          "NoResultAndNoParameters",
          DataType::None, 
          ParameterTypes{},
          make_exception_ptr(logic_error("Response should not return values")),
          Parameters{}
        },
        CallableTestParam{
          "BoolResultWithoutParameters",
          DataType::Boolean, 
          ParameterTypes{},
          true,
          Parameters{}
        },
        CallableTestParam{
          "IntegerResultWithoutParameters",
          DataType::Integer, 
          ParameterTypes{},
          (intmax_t)-11,
          Parameters{}
        },
        CallableTestParam{
          "UIntegerResultWithoutParameters",
          DataType::Unsigned_Integer, 
          ParameterTypes{},
          (uintmax_t)25,
          Parameters{}
        },
        CallableTestParam{
          "DoubleResultWithoutParameters",
          DataType::Double, 
          ParameterTypes{},
          30.2,
          Parameters{}
        },
        CallableTestParam{
          "TimeResultWithoutParameters",
          DataType::Timestamp, 
          ParameterTypes{},
          Timestamp{2025, 9, 11, 10, 01, 24, 32},
          Parameters{}
        },
        CallableTestParam{
          "OpaqueResultWithoutParameters",
          DataType::Opaque, 
          ParameterTypes{},
          vector<uint8_t>{0x00,0x01,0x02},
          Parameters{}
        },
        CallableTestParam{
          "StringResultWithoutParameters",
          DataType::String, 
          ParameterTypes{},
          "hello world",
          Parameters{}
        }
    ), // clang-format on
    [](const TestParamInfo<CallableTests::ParamType>& info) {
      return info.param.test_name;
    });
// NOLINTEND(readability-magic-numbers)
} // namespace Information_Model::testing