# Using Mocks {#mock_usage}

## Setting call expectations

It is a good practice to set method call expectations when using mocks (however it is not necessary when using mocks as dummy data), since it add more quality checks to your unit tests cases and provides better validation of your test scenarios. Since these mocks are based on GTest mocking module, all of the Gtest mocking mechanisms apply here as well.

```cpp
#include <Information_Model_Mock/WritableMock.hpp>

#include <memory>

namespace Information_Model::testing {
using namespace ::testing; // import gtest namespace

auto mock = std::make_shared<WritableMock>(DataType::Double);

// Create an expectation that the mock will get a write call with 20.2 value
// exactly once
EXPECT_CALL(*mock, write(20.2)).Times(Exactly(1));

// Fulfill the expectation
// It is usually done in a separate code block, that is not part of the test
mock->write(20.2);
} // namespace Information_Model::testing
```

For more detailed information see [GTest Expectation doc](https://google.github.io/googletest/reference/mocking.html#EXPECT_CALL)

## Using Observable mocks

Using Observable mocks can be unintuitive, if you are not familiar with the [*Observer*](https://archive.org/details/designpatternsel00gamm/page/292/mode/2up) software pattern. 

This following example shows how you can subscribe to the Observable mock, process and dispatch notifications. 

```cpp
#include <Information_Model_Mock/ObservableMock.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace Information_Model::testing {
void handleException(const std::exception_ptr& ex_ptr) {
  // handle any thrown exception from the observer
}

// Forward declaration for startSendingNotifications function
ObservableMockPtr observable;

// This function is used by the Observable implementation to indicate when to
// start/stop sending notifications. This function should not be blocking! We only make it blocking to keep this example simple and short. Dispatching notifications should be done in a separate thread, thread pool or asynchronously
void startSendingNotifications(bool enable) {
  using namespace std::literals::chrono_literals;
   
  if (enable) {
    // start dispatching notifications periodically, or when a new value comes
    if (observable) {
      // to dispatch notifications to observers use ObservableMock::notify()
      observable->notify("Hello observers");
      std::this_thread::sleep_for(10ms);
      observable->notify("Sending a new value");
    } else {
      throw logic_error("Oh no, observable no longer exists");
    }
  } else {
    // stop dispatching notifications, since there are no more listeners waiting
    // for notifications
  }
}

observable = std::make_shared<ObservableMock>(DataType::String);
observable->enableSubscribeFaking(&startSendingNotifications);

// When the first observer is created, startSendingNotifications(true) is called
auto cout_observer = observable->subscribe(
    [](const DataVariantPtr& notification) {
      std::cout << "Cout observer received: "
                << std::get<std::string>(*notification) << std::endl;
    },
    &handleException);

// You need to keep the resulting ObserverPtr alive for as long as you want to
// receive notification
auto cerr_observer = observable->subscribe(
    [](const DataVariantPtr& notification) {
      std::cerr << "Cerr observer received: "
                << std::get<std::string>(*notification) << std::endl;
    },
    &handleException);

observable->notify("Sending another value");

// Destroying the ObserverPtr, unsubscribes from notifications
cout_observer.reset();

observable->notify("Only Cerr sees this");

// Destroing the last observer, calls startSendingNotifications(false)
cerr_observer.reset();

} // namespace Information_Model::testing
```

## Using Callable mocks

Using Callable mocks can be a difficult task, if you are using your own callbacks, since you need to keep track of the assigned `ResultFuture` instances and their promises, provide safety mechanisms for multithreading, as well as asynchronous call execution. To make this task simpler, we provide an `Executor` which handles all of these problems for you.

```cpp
#include <Information_Model_Mock/CallableMock.hpp>

#include <iostream>
#include <memory>

namespace Information_Model::testing {
auto callable = std::make_shared<CallableMock>(DataType::Integer);

// get the executor
auto executor = callable->getExecutor();

// start the auto response mechanism
executor->start();

auto result = callable->call(100);

// stop the auto response mechanism
executor->stop();

// You can queue up responses for future asyncCall() invocations
executor->queueResponse(20);
executor->queueResponse(30);
executor->queueResponse( // you can also queue up exceptions
    3, std::make_exception_ptr(std::runtime_error("Evil error")));

auto result_future_0 = callable->asyncCall(); // will get 20 value
auto result_future_1 = callable->asyncCall(); // will get 30 value
auto result_future_2 = callable->asyncCall(); // will get default integer value
auto result_future_3 = callable->asyncCall(); // will throw runtime_error

// dispatch the queued up responses to the allocated result futures
for (uint8_t i = 0; i < 4; i++) {
  // You can manually dispatch queued up responses
  executor->respondOnce();
}

std::cout << "Request 0 result: " << toString(result_future_0.get())
          << std::endl;
std::cout << "Request 1 result: " << toString(result_future_1.get())
          << std::endl;
std::cout << "Request 1 result: " << toString(result_future_2.get())
          << std::endl;
try {
  std::cerr << "Request 3 result should not return a value but returns: "
            << toString(result_future_3.get()) << std::endl;
} catch (const std::runtime_error& ex) {
  std::cout << "Request 3 threw exception" << ex.waht() " as expected"
            << std::endl;
}

auto result_future = callable->asyncCall();

// You can also respond to a specific request via it's id
executor->respond(result_future.id(), 333);

std::cout << "Request " << result_future.id() " result: " << result_future.get()
          << std::endl;
} // namespace Information_Model::testing
```

## Leak still reachable valgrind error for Nice and Strict mocks

As explained in googletest github issue [[Bug]: valgrind reports still reachable memory leaks when StrictMock or NiceMock are used #4109](https://github.com/google/googletest/issues/4109#issuecomment-1376362854) using the [NiceMock](https://google.github.io/googletest/reference/mocking.html#NiceMock) or [StrictMock](https://google.github.io/googletest/reference/mocking.html#StrictMock) decorators causes valgrind memory analysis to report `UninterestingCallReactionMap` and it's related hash table to be still reachable in the loss record. This behavior is expected and should not cause alarm. However having false positive result during memory analysis is not really desired. For this reason, this project provides a `valgrind.supp` file in the root of this project, which tells valgrind, which symbols should be ignored during analysis.

In general cases, you should be fine reusing the `valgrind.supp` file this project provides, but in case you need to make your own, you can generate a the content of your new suppression file by running the following command: 

```bash
valgrind --leak-check=full --num-callers=500 --fair-sched=try --show-reachable=yes --gen-suppressions=all ${TARGET_EXECUTABLE} &> >(tee valgrind.log)
```

This will dump the valgrind output with the suppression rules into a `valgrind.log` file in your current work directory, however you will have to manually edit this file to only contain `{<insert_a_suppression_name_here>...}` text blocks. Furthermore you will have to verify that you are saving only the desired valgrind suppression rules instead of suppressing all vaglrind errors.

You can find the relevant information on suppressing valgrind errors in the Valgrind manual section [2.5 Suppressing errors](https://valgrind.org/docs/manual/manual-core.html) or as a quick and simple summery in [wxWiki](https://wiki.wxwidgets.org/Valgrind_Suppression_File_Howto)
