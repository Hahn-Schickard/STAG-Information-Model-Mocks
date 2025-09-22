#ifndef __STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP
#include <Information_Model/Callable.hpp>

namespace Information_Model::testing {

struct Executor {
  using Response = std::variant<DataVariant, std::exception_ptr>;

  virtual ~Executor() = default;

  /**
   * @brief Emulates the execution of a function that returns no result.
   *
   * Used by CallableMock::execute(const Parameters&) invocations
   *
   * @attention This method is intended to set GMock expectations and should not
   * be directly used by the user
   *
   * @see
   * [EXPECT_CALL](https://google.github.io/googletest/reference/mocking.html#EXPECT_CALL)
   * for reference
   *
   * @param params
   */
  virtual void execute(const Parameters& params) = 0;

  /**
   * @brief Emulates the execution of a function that allocates a ResultFuture
   * as a result
   *
   * Used by CallableMock::call(uintmax_t), CallableMock::call(const
   * Parameters&, uintmax_t) and CallableMock::asyncCall(const Parameters&)
   * invocations
   *
   * @attention This method is intended to set GMock expectations and should not
   * be directly used by the user
   *
   * @see
   * [EXPECT_CALL](https://google.github.io/googletest/reference/mocking.html#EXPECT_CALL)
   * for reference
   *
   * @param params
   * @return ResultFuture
   */
  [[nodiscard]] virtual ResultFuture asyncCall(const Parameters& params) = 0;

  /**
   * @brief Dispatches the cancel exception for the given asyncCall ResultFuture
   * instance. Does nothing if no linked ResultFuture exist
   *
   * Used by CallableMock::cancelAsyncCall(uintmax_t) invocations
   *
   * @attention This method is intended to set GMock expectations and should not
   * be directly used by the user
   *
   * @see
   * [EXPECT_CALL](https://google.github.io/googletest/reference/mocking.html#EXPECT_CALL)
   * for reference
   *
   * @param call_id
   */
  virtual void cancel(uintmax_t call_id) = 0;

  /**
   * @brief Dispatched the cancel exception to all currently linked  asyncCall
   * ResultFuture instances
   *
   * Used by CallableMock::changeExecutor() or when this Executor instance is
   * destroyed
   *
   * @attention This method is ment for cleanup operations and is not generally
   * intended for testing purposes
   */
  virtual void cancelAll() = 0;

  /**
   * @brief Emulate the resultType() getter method
   *
   * Used by CallableMock::resultType() invocations
   *
   * @attention This method is intended to set GMock expectations and should
   * not be directly used by the user
   *
   * @see
   * [EXPECT_CALL](https://google.github.io/googletest/reference/mocking.html#EXPECT_CALL)
   * for reference
   *
   * @return DataType
   */
  virtual DataType resultType() const = 0;

  /**
   * @brief Emulate the parameterTypes() getter method
   *
   * Used by CallableMock::parameterTypes() invocations
   *
   * @attention This method is intended to set GMock expectations and should
   * not be directly used by the user
   *
   * @see
   * [EXPECT_CALL](https://google.github.io/googletest/reference/mocking.html#EXPECT_CALL)
   * for reference
   *
   * @return ParameterTypes
   */
  virtual ParameterTypes parameterTypes() const = 0;

  /**
   * @brief Respond with a given response value to a given
   * CallableMock::call(uintmax_t), CallableMock::call(const Parameters&,
   * uintmax_t) or CallableMock::asyncCall(const Parameters&) ResultFuture
   * instance
   *
   * @attention This method should not be used when start() method was called
   *
   * @throws CallerNotFound - if no ResultFuture exists for a given call id
   *
   * @param call_id
   * @param response
   */
  virtual void respond(uintmax_t call_id, const Response& response) = 0;

  /**
   * @brief Enqueue a given response for the next CallableMock::call(uintmax_t),
   * CallableMock::call(const Parameters&, uintmax_t) or
   * CallableMock::asyncCall(const Parameters&) ResultFuture instance invocation
   *
   * @param response
   */
  virtual void queueResponse(const Response& response) = 0;

  /**
   * @brief Enqueue a given response for a specific
   * CallableMock::call(uintmax_t), CallableMock::call(const Parameters&,
   * uintmax_t) or CallableMock::asyncCall(const Parameters&) ResultFuture
   * instance invocation
   *
   * @param call_id
   * @param response
   */
  virtual void queueResponse(uintmax_t call_id, const Response& response) = 0;

  /**
   * @brief Dispatch a response to the next queued request and cleanup any freed
   * up call ids. If no new request has been queued up with
   * CallableMock::call(uintmax_t), CallableMock::call(const Parameters&,
   * uintmax_t) or CallableMock::asyncCall(const Parameters&) invocations, runs
   * only the call id cleanup procedure
   *
   */
  virtual void respondOnce() = 0;

  /**
   * @brief Starts automatically responding to any
   * CallableMock::call(uintmax_t), CallableMock::call(const Parameters&,
   * uintmax_t) or CallableMock::asyncCall(const Parameters&) invocations
   *
   */
  virtual void start() = 0;

  /**
   * @brief Stops automatically responding to any
   * CallableMock::call(uintmax_t), CallableMock::call(const Parameters&,
   * uintmax_t) or CallableMock::asyncCall(const Parameters&) invocations
   *
   */
  virtual void stop() = 0;
};

using ExecutorPtr = std::shared_ptr<Executor>;

ExecutorPtr makeExecutor(DataType result_type,
    const ParameterTypes& supported_params,
    const Executor::Response& default_response, std::chrono::nanoseconds delay);

} // namespace Information_Model::testing

#endif //__STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP