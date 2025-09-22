#ifndef __STAG_INFORMATION_MODEL_MOCKS_OBSERVABLE_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_OBSERVABLE_MOCK_HPP
#include "ReadableMock.hpp"

#include <Information_Model/Observable.hpp>
#include <gmock/gmock.h>

#include <mutex>
#include <vector>

namespace Information_Model::testing {

struct ObserverPimpl : virtual public Observer {
  ~ObserverPimpl() override = default;

  virtual void dispatch(const std::shared_ptr<DataVariant>& value) = 0;
};

struct ObservableMock : public Observable {
  using ReadCallback = ReadableMock::ReadCallback;
  using IsObservingCallback = std::function<void(bool)>;

  ObservableMock() = default;

  explicit ObservableMock(DataType type);

  explicit ObservableMock(const DataVariant& value);

  ObservableMock(DataType type, const ReadCallback& read_cb);

  ~ObservableMock() override = default;

  /**
   * @brief Enables or disables the usage of the internal Observer tracker
   *
   * If a given callback is not null, enables internal Observer tracking and
   * notification mechanism. If the given callback is null, configures the
   * subscribe() method to return a dummy ObserverPtr instance that has no
   * attached notifier (Dummy Observers will not be notified when notify() is
   * called)
   *
   * @param callback
   */
  void enableSubscribeFaking(const IsObservingCallback& callback);

  /**
   * @brief Change the modeled data type
   *
   * Same as @ref ReadableMock::updateType()
   *
   * @param type
   */
  void updateType(DataType type);

  /**
   * @brief Change the default read() result
   *
   * Same as @ref ReadableMock::updateValue()
   *
   * @param value
   */
  void updateValue(const DataVariant& value);

  /**
   * @brief Change the internal callback that is used for read() invocations
   * If the given callback is set to nullptr, forces the read() calls to throw
   * ReadCallbackUnavailable exception
   *
   * Same as @ref ReadableMock::updateReadCallback()
   *
   * @param read_cb
   */
  void updateReadCallback(const ReadCallback& read_cb);

  /**
   * @brief Dispatch a new notification value to all registered Observers
   * (Does nothing if enableSubscribeFaking() was never called or the last
   * enableSubscribeFaking() call passed a nullptr parameter value)
   *
   * @param value
   */
  void notify(const DataVariant& value);

  MOCK_METHOD(DataType, dataType, (), (const final));
  MOCK_METHOD(DataVariant, read, (), (const final));
  MOCK_METHOD(ObserverPtr, subscribe,
      (const Observable::ObserveCallback&, const Observable::ExceptionHandler&),
      (final));

private:
  void setReadableCalls() const;

  ObserverPtr attachObserver(const Observable::ObserveCallback& callback,
      const Observable::ExceptionHandler& handler);

  ReadableMockPtr readable_;
  std::mutex mx_;
  IsObservingCallback is_observing_;
  std::vector<std::weak_ptr<ObserverPimpl>> observers_;
};

using ObservableMockPtr = std::shared_ptr<ObservableMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_OBSERVABLE_MOCK_HPP