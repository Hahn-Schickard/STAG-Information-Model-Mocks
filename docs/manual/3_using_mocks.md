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

```cpp
```

## Leak still reachable valgrind error for Nice and Strict mocks

As explained in googletest github issue [[Bug]: valgrind reports still reachable memory leaks when StrictMock or NiceMock are used #4109](https://github.com/google/googletest/issues/4109#issuecomment-1376362854) using the [NiceMock](https://google.github.io/googletest/reference/mocking.html#NiceMock) or [StrictMock](https://google.github.io/googletest/reference/mocking.html#StrictMock) decorators causes valgrind memory analysis to report `UninterestingCallReactionMap` and it's related hash table to be still reachable in the loss record. This behavior is expected and should not cause alarm. However having false positive result during memory analysis is not really desired. For this reason, this project provides a `valgrind.supp` file in the root of this project, which tells valgrind, which symbols should be ignored during analysis.

In general cases, you should be fine reusing the `valgrind.supp` file this project provides, but in case you need to make your own, you can find the relevant information on suppressing valgrind errors in the Valgrind manual section [2.5 Suppressing errors](https://valgrind.org/docs/manual/manual-core.html) or as a quick and simple summery in [wxWiki](https://wiki.wxwidgets.org/Valgrind_Suppression_File_Howto)
