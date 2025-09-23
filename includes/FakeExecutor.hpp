#ifndef __STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP
#include <Information_Model/Callable.hpp>

namespace Information_Model::testing {

struct Executor {
  using Response = std::variant<DataVariant, std::exception_ptr>;

  virtual ~Executor() = default;

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

private:
  virtual void execute(const Parameters& params) = 0;

  [[nodiscard]] virtual ResultFuture asyncCall(const Parameters& params) = 0;

  virtual void cancel(uintmax_t call_id) = 0;

  virtual void cancelAll() = 0;

  virtual DataType resultType() const = 0;

  virtual ParameterTypes parameterTypes() const = 0;

  friend struct CallableMock;
};

using ExecutorPtr = std::shared_ptr<Executor>;

ExecutorPtr makeExecutor(DataType result_type,
    const ParameterTypes& supported_params,
    const Executor::Response& default_response, std::chrono::nanoseconds delay);

} // namespace Information_Model::testing

#endif //__STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP