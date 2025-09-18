#include "FakeExecutor.hpp"

#include <Stoppable/Task.hpp>
#include <Variant_Visitor/Visitor.hpp>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace Information_Model::testing {
using namespace std;

struct IdRepository {
  shared_ptr<uintmax_t> assignID() {
    auto id = make_shared<uintmax_t>(0);
    while (!ids_.try_emplace(*id, id).second) {
      (*id)++;
    }
    return id;
  }

  void freeIDs() {
    for (auto it = ids_.begin(); it != ids_.end();) {
      if (it->second.expired()) {
        it = ids_.erase(it);
      } else {
        ++it;
      }
    }
  }

private:
  unordered_map<uintmax_t, weak_ptr<uintmax_t>> ids_;
};

struct DispatchQueue {
  void enqueue(uintmax_t call_id) {
    scoped_lock lock(mx_);
    queue_.push(call_id);
    queue_not_empty_.notify_one();
  }

  optional<uintmax_t> dequeue() {
    unique_lock lock(mx_);
    if (queue_not_empty_.wait_for(
            lock, 100us, [this]() { return !queue_.empty(); })) {
      auto dispatch_id = queue_.front();
      queue_.pop();
      return dispatch_id;
    } else {
      return nullopt;
    }
  }

private:
  mutex mx_;
  condition_variable queue_not_empty_;
  queue<uintmax_t> queue_;
};

struct ResponseRepository {
  using Response = Executor::Response;

  explicit ResponseRepository(const Response& default_response)
      : default_(default_response) {}

  void enqueue(const Response& response) { queue_.push(response); }

  void emplace(uintmax_t id, const Response& response) {
    map_.try_emplace(id, response);
  }

  Response get(uintmax_t id) {
    if (auto it = map_.find(id); it != map_.end()) {
      auto response = it->second;
      it = map_.erase(it);
      return response;
    } else if (!queue_.empty()) {
      auto response = queue_.front();
      queue_.pop();
      return response;
    } else {
      return default_;
    }
  }

private:
  Response default_;
  queue<Response> queue_;
  unordered_map<uintmax_t, Response> map_;
};

struct FakeExecutor : public Executor {
  FakeExecutor() = default;

  FakeExecutor(DataType result_type,
      const ParameterTypes& supported,
      const Executor::Response& default_response,
      chrono::nanoseconds response_delay)
      : result_type_(result_type), supported_params_(supported),
        responses_(ResponseRepository(default_response)),
        delay_(response_delay),
        task_(make_shared<Stoppable::Task>(
            bind(&FakeExecutor::respondOnce, this), [](const exception_ptr&) {
              // suppress any thrown exception
            })) {}

  ~FakeExecutor() override { cancelAll(); }

  void delayCall() const {
    if (delay_.count() > 0) {
      this_thread::sleep_for(delay_);
    }
  }

  void execute(const Parameters& params) final {
    delayCall();
    checkParameters(params, supported_params_);
  }

  ResultFuture asyncCall(const Parameters& params) final {
    if (result_type_ == DataType::None) {
      throw ResultReturningNotSupported();
    }
    auto call_id = id_repo_.assignID();
    promise<DataVariant> result_promise{};
    try {
      checkParameters(params, supported_params_);
    } catch (...) {
      result_promise.set_exception(current_exception());
      return ResultFuture(call_id, result_promise.get_future());
    }
    ResultFuture result_future(call_id, result_promise.get_future());
    result_promises_.try_emplace(*call_id, move(result_promise));
    dispatch_queue_.enqueue(*call_id);
    return result_future;
  }

  void respond(uintmax_t call_id, const Response& response) final {
    delayCall();
    auto it = result_promises_.find(call_id);
    if (it != result_promises_.end()) {
      Variant_Visitor::match(
          response,
          [&it](const DataVariant& value) { it->second.set_value(value); },
          [&it](const exception_ptr& exception) {
            it->second.set_exception(exception);
          });
      result_promises_.erase(it);
    } else {
      throw CallerNotFound(call_id, "ExternalExecutor");
    }
  }

  void cancel(uintmax_t call_id) final {
    if (auto it = result_promises_.find(call_id);
        it != result_promises_.end()) {
      it->second.set_exception(
          make_exception_ptr(CallCanceled(call_id, "MockCallable")));
      it = result_promises_.erase(it);
    }
  }

  void cancelAll() final {
    for (auto& [promise_id, result_promise] : result_promises_) {
      result_promise.set_exception(
          make_exception_ptr(CallCanceled(promise_id, "MockCallable")));
    }
    result_promises_.clear();
  }

  DataType resultType() const final { return result_type_; }

  ParameterTypes parameterTypes() const final { return supported_params_; }

  void checkType(const Response& response) const {
    if (holds_alternative<DataVariant>(response)) {
      if (result_type_ == DataType::None) {
        throw invalid_argument("Can not set DataVariant response for executor. "
                               "Executor does not support returning values");
      }
      if (result_type_ != toDataType(get<DataVariant>(response))) {
        throw invalid_argument("Executor is suppose to return " +
            toString(result_type_) + " data values, not " +
            toString(toDataType(get<DataVariant>(response))));
      }
    }
  }

  void queueResponse(const Response& response) final {
    checkType(response);
    responses_.enqueue(response);
  }

  void queueResponse(uintmax_t call_id, const Response& response) final {
    checkType(response);
    responses_.emplace(call_id, response);
  }

  void respondOnce() final {
    if (auto next_dispatch = dispatch_queue_.dequeue()) {
      auto response_id = next_dispatch.value();
      respond(response_id, responses_.get(response_id));
    }
    id_repo_.freeIDs();
  }

  void start() final { task_->start(); }

  void stop() final { task_->stop(); }

private:
  DataType result_type_ = DataType::None;
  ParameterTypes supported_params_;
  ResponseRepository responses_;
  chrono::nanoseconds delay_;
  Stoppable::TaskPtr task_;
  IdRepository id_repo_;
  DispatchQueue dispatch_queue_;
  unordered_map<uintmax_t, promise<DataVariant>> result_promises_;
};

ExecutorPtr makeExecutor(DataType result_type,
    const ParameterTypes& supported_params,
    const Executor::Response& default_response,
    chrono::nanoseconds delay) {
  return make_shared<FakeExecutor>(
      result_type, supported_params, default_response, delay);
}
} // namespace Information_Model::testing
