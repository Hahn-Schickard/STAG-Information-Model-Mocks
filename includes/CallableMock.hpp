#ifndef __STAG_INFORMATION_MODEL_MOCKS_CALLABLE_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_CALLABLE_MOCK_HPP

#include "FakeExecutor.hpp"

#include <gmock/gmock.h>

namespace Information_Model::testing {

struct CallableMock : public Callable {
  using ExecuteCallback = std::function<void(const Parameters&)>;
  using AsyncExecuteCallback = std::function<ResultFuture(const Parameters&)>;
  using CancelCallback = std::function<void(uintmax_t)>;

  CallableMock() = default;

  explicit CallableMock(const ExecutorPtr& executor);

  explicit CallableMock(DataType result_type,
      const ParameterTypes& supported_params = {},
      const Executor::Response& default_response = std::make_exception_ptr(
          std::logic_error("Default response exception")));

  explicit CallableMock(const ExecuteCallback& execute_cb,
      const ParameterTypes& supported_params = {});

  CallableMock(DataType result_type, const ExecuteCallback& execute_cb,
      const AsyncExecuteCallback& async_execute_cb,
      const CancelCallback& cancel_cb,
      const ParameterTypes& supported_params = {});

  ~CallableMock() override = default;

  MOCK_METHOD(void, execute, (const Parameters& parameters), (const final));
  MOCK_METHOD(DataVariant, call, (uintmax_t timeout), (const final));
  MOCK_METHOD(DataVariant, call,
      (const Parameters& parameters, uintmax_t timeout), (const final));
  MOCK_METHOD(
      ResultFuture, asyncCall, (const Parameters& parameters), (const final));
  MOCK_METHOD(void, cancelAsyncCall, (uintmax_t), (const final));
  MOCK_METHOD(DataType, resultType, (), (const final));
  MOCK_METHOD(ParameterTypes, parameterTypes, (), (const final));

  /**
   * @brief Returns the configured Executor instance
   *
   * @throws std::logic_error - if external callbacks are used
   *
   * @return ExecutorPtr
   */
  ExecutorPtr getExecutor() const;

  /**
   * @brief Changes the configured executor or external callback usage to the
   * given executor. If given executor is null, sets CallableMock::execute(),
   * CallableMock::call(), CallableMock::asyncCall() and
   * CallableMock::cancelAsynCall() methods to throw ExecutorNotAvailable
   * exception when called
   *
   * @param executor
   */
  void changeExecutor(const ExecutorPtr& executor);

  /**
   * @brief Creates a default executor instance based on modeled result type and
   * supported parameters. Overrides any previous executor or external callback
   * links
   *
   */
  void useDefaultExecutor();

  /**
   * @brief Resets this mock to used initially provided external callbacks
   *
   * @throws std::logic_error - if no external callback were set during
   * construction
   *
   */
  void useDefaultCallbacks();

private:
  void setExecutor();
  void setCallbacks();

  DataType result_type_;
  ExecuteCallback execute_cb_;
  AsyncExecuteCallback async_execute_cb_;
  CancelCallback cancel_cb_;
  ParameterTypes supported_params_;
  Executor::Response default_response_;
  ExecutorPtr executor_;
};

using CallableMockPtr = std::shared_ptr<CallableMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_CALLABLE_MOCK_HPP