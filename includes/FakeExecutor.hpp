#ifndef __STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP
#include <Information_Model/Callable.hpp>

namespace Information_Model::testing {

struct Executor {
  using Response = std::variant<DataVariant, std::exception_ptr>;

  virtual ~Executor() = default;

  virtual void execute(const Parameters& params) = 0;

  [[nodiscard]] virtual ResultFuture asyncCall(const Parameters& params) = 0;

  virtual void cancel(uintmax_t call_id) = 0;

  virtual void cancelAll() = 0;

  virtual DataType resultType() const = 0;

  virtual ParameterTypes parameterTypes() const = 0;

  virtual void respond(uintmax_t call_id, const Response& response) = 0;

  virtual void queueResponse(const Response& response) = 0;

  virtual void queueResponse(uintmax_t call_id, const Response& response) = 0;

  virtual void respondOnce() = 0;

  virtual void start() = 0;

  virtual void stop() = 0;
};

using ExecutorPtr = std::shared_ptr<Executor>;

ExecutorPtr makeExecutor(DataType result_type,
    const ParameterTypes& supported_params,
    const Executor::Response& default_response, std::chrono::nanoseconds delay);

} // namespace Information_Model::testing

#endif //__STAG_INFORMATION_MODEL_MOCKS_EXECUTOR_MOCK_HPP