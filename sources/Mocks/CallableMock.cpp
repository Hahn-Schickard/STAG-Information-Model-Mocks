#include "CallableMock.hpp"

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

CallableMock::CallableMock(const ExecutorPtr& executor) : executor_(executor) {
  setExecutor();
}

CallableMock::CallableMock(DataType result_type,
    const ParameterTypes& supported_params,
    const Executor::Response& default_response)
    : result_type_(result_type), supported_params_(supported_params),
      default_response_(default_response),
      executor_(makeExecutor(
          result_type_, supported_params_, default_response_, 100ms)) {
  setExecutor();
}

CallableMock::CallableMock(
    const ExecuteCallback& execute_cb, const ParameterTypes& supported_params)
    : result_type_(DataType::None), execute_cb_(execute_cb),
      supported_params_(supported_params) {
  setCallbacks();
}

CallableMock::CallableMock(DataType result_type,
    const ExecuteCallback& execute_cb,
    const AsyncExecuteCallback& async_execute_cb,
    const CancelCallback& cancel_cb,
    const ParameterTypes& supported_params)
    : result_type_(result_type), execute_cb_(execute_cb),
      async_execute_cb_(async_execute_cb), cancel_cb_(cancel_cb),
      supported_params_(supported_params) {
  setCallbacks();
}

ExecutorPtr CallableMock::getExecutor() const {
  if (!executor_) {
    throw logic_error("External callbacks are used instead of the executor");
  }
  return executor_;
}

void CallableMock::changeExecutor(const ExecutorPtr& executor) {
  if (executor_) {
    executor_->cancelAll();
  }

  executor_ = executor;

  setExecutor();
}

void CallableMock::useDefaultExecutor() {
  executor_ =
      makeExecutor(result_type_, supported_params_, default_response_, 100ms);
  setExecutor();
}

void CallableMock::setExecutor() {
  if (executor_) {
    ON_CALL(*this, resultType)
        .WillByDefault(Invoke(executor_.get(), &Executor::resultType));
    ON_CALL(*this, parameterTypes)
        .WillByDefault(Invoke(executor_.get(), &Executor::parameterTypes));
    ON_CALL(*this, execute).WillByDefault([this](const Parameters& params) {
      executor_->execute(params);
    });
    ON_CALL(*this, call(_)).WillByDefault([this](uintmax_t timeout) {
      auto result = executor_->asyncCall(makeDefaultParams(supported_params_));
      auto status = result.waitFor(chrono::milliseconds(timeout));
      if (status == future_status::ready) {
        return result.get();
      } else {
        throw CallTimedout("CallableMock Executor");
      }
    });
    ON_CALL(*this, call(_, _))
        .WillByDefault([this](const Parameters& params, uintmax_t timeout) {
          auto result = executor_->asyncCall(params);
          auto status = result.waitFor(chrono::milliseconds(timeout));
          if (status == future_status::ready) {
            return result.get();
          } else {
            throw CallTimedout("CallableMock Executor");
          }
        });
    ON_CALL(*this, asyncCall)
        .WillByDefault(bind(&Executor::asyncCall, executor_, placeholders::_1));
    ON_CALL(*this, cancelAsyncCall)
        .WillByDefault(bind(&Executor::cancel, executor_, placeholders::_1));
  } else {
    ON_CALL(*this, resultType).WillByDefault(Return(result_type_));
    ON_CALL(*this, parameterTypes).WillByDefault(Return(supported_params_));
    ON_CALL(*this, execute).WillByDefault(Throw(ExecutorNotAvailable()));
    ON_CALL(*this, call(_)).WillByDefault(Throw(ExecutorNotAvailable()));
    ON_CALL(*this, call(_, _)).WillByDefault(Throw(ExecutorNotAvailable()));
    ON_CALL(*this, asyncCall).WillByDefault(Throw(ExecutorNotAvailable()));
    ON_CALL(*this, cancelAsyncCall)
        .WillByDefault(Throw(ExecutorNotAvailable()));
  }
}

void CallableMock::useDefaultCallbacks() {
  if (!execute_cb_) {
    throw logic_error("Default callbacks not set");
  }
  setCallbacks();
}

void CallableMock::setCallbacks() {
  ON_CALL(*this, resultType).WillByDefault(Return(result_type_));
  ON_CALL(*this, parameterTypes).WillByDefault(Return(supported_params_));
  ON_CALL(*this, execute).WillByDefault(execute_cb_);

  if (result_type_ != DataType::None) {
    ON_CALL(*this, call(_)).WillByDefault([this](uintmax_t timeout) {
      auto result = executor_->asyncCall(makeDefaultParams(supported_params_));
      auto status = result.waitFor(chrono::milliseconds(timeout));
      if (status == future_status::ready) {
        return result.get();
      } else {
        throw CallTimedout("External Executor");
      }
    });
    ON_CALL(*this, call(_, _))
        .WillByDefault([this](const Parameters& params, uintmax_t timeout) {
          auto result = async_execute_cb_(params);
          auto status = result.waitFor(chrono::milliseconds(timeout));
          if (status == future_status::ready) {
            return result.get();
          } else {
            throw CallTimedout("External Executor");
          }
        });
    ON_CALL(*this, asyncCall).WillByDefault(async_execute_cb_);
    ON_CALL(*this, cancelAsyncCall).WillByDefault(cancel_cb_);
  } else {
    ON_CALL(*this, call(_)).WillByDefault(Throw(ResultReturningNotSupported()));
    ON_CALL(*this, call(_, _))
        .WillByDefault(Throw(ResultReturningNotSupported()));
    ON_CALL(*this, asyncCall)
        .WillByDefault(Throw(ResultReturningNotSupported()));
    ON_CALL(*this, cancelAsyncCall)
        .WillByDefault(Throw(ResultReturningNotSupported()));
  }
}

} // namespace Information_Model::testing