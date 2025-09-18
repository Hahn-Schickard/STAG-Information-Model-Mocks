#include "MockBuilder.hpp"

#include <Variant_Visitor/Visitor.hpp>

#include <functional>
#include <iostream>

using namespace std;
using namespace Information_Model;

void printElement(const ElementPtr& element, size_t padding) {
  cout << string(padding, ' ') << "Element: {" << element->id() << "} named: {"
       << element->name() << "} and described as {" << element->description()
       << "} This element ";
  Variant_Visitor::match(
      element->function(),
      [](const ReadablePtr& readable) {
        cout << "can read " << toString(readable->dataType()) << ": "
             << toString(readable->read()) << " value" << endl;
      },
      [](const WritablePtr& writable) {
        if (writable->isWriteOnly()) {
          cout << "can only write " << toString(writable->dataType())
               << " values" << endl;
        } else {
          cout << "can read and write " << toString(writable->dataType())
               << " values. It currently reads " << toString(writable->read())
               << " value" << endl;
        }
      },
      [](const ObservablePtr& observable) {
        cout << "can observe " << toString(observable->dataType())
             << " values. Current value is: " << toString(observable->read())
             << " value" << endl;
      },
      [](const CallablePtr& callable) {
        cout << "can execute operations that accept "
             << toString(callable->parameterTypes()) << " parameters";
        if (callable->resultType() == DataType::None) {
          cout << " and returns no value" << endl;
        } else {
          cout << " and returns " << toString(callable->resultType())
               << " value" << endl;
        }
      },
      [&padding](const GroupPtr& group) {
        cout << "groups other elements. It contains: " << group->size()
             << " elements as follows: [" << endl;
        padding += 2;
        group->visit([&padding](const ElementPtr& sub_element) {
          printElement(sub_element, padding);
        });
        padding -= 2;
        cout << string(padding, ' ') << "]" << endl;
      });
}

void printDevice(const DevicePtr& device) {
  cout << "Device {" << device->id() << "} named: {" << device->name()
       << "} and described as {" << device->description() << "} has "
       << device->size() << " elements. These elements are as follow: ["
       << endl;
  device->visit([](const ElementPtr& element) { printElement(element, 1); });
  cout << "]" << endl;
}

DevicePtr makeDevice();

int main() {
  try {
    // Build example device
    auto device = makeDevice();

    // Print out it's contnet
    printDevice(device);
  } catch (const exception& ex) {
    cerr << "Example runner encountered an exception: " << ex.what() << endl;
  }

  return 0;
}

DataVariant readCallback() {
  // Your read operation
  return "Hello World";
}

void executeCallback(const Parameters&) {
  // Your execute operation, it MUsT NOT block the caller
}

ResultFuture asyncExecuteCallback(const Parameters&) {
  // Your async execute operation
  // The promise is your placeholder buffer for the async result value
  auto promised_result = promise<DataVariant>();
  shared_ptr<uintmax_t> caller_id; // this id is used to cancel async operation
  {
    // the caller ID value SHOULD be unique to avoid canceling newer calls
    caller_id = make_shared<uintmax_t>(0);
    // you should set the value in a separate operation, the one that is
    // fulfilling the promise, (most likely in another thread), but for brevity,
    // we are setting the value at the same time we are creating the promise
    promised_result.set_value("Hello from async!");
  }
  // ResultFuture fully owns both the caller_id and the std::future objects
  return ResultFuture(move(caller_id), promised_result.get_future());
}

void cancelAsyncExecuteCallback(uintmax_t) {
  // Your cancel async execute operation
  /* Find the promised result and set CallCanceled exception to indicate the
    call was canceled
     promised_result.set_exception(
      make_exception_ptr(CallCanceled(caller_id, "[CALLBACK]: User canceled")));
  */
  // If you cant find the promised result, throw CallerNotFound exception
  // to the caller directly
}

void enableObservationCallback(bool) {
  // Your enable observation callback
  // This operation is called to indicate when you should start/stop calling the
  // DeviceBuilder::NotifyCallback callable
  // This operation MUST NOT block the caller indefinitely
}

DevicePtr makeDevice() {
  using namespace Information_Model::testing;

  auto builder = make_shared<MockBuilder>();

  builder->setDeviceInfo(
      "12345Example", BuildInfo{"Example", "Device example"});

  builder->addReadable(BuildInfo{"readable", "C style callback"},
      DataType::String,
      &readCallback);

  builder->addWritable(
      BuildInfo{"writable", "C++ lambda and std::bind callbacks"},
      DataType::String,
      [](const DataVariant& value) {
        cout << "[CALLBACK] Writing value: " << toString(value) << endl;
      },
      bind(&readCallback));

  [[maybe_unused]] auto [observable_id, // this holds the built element id
      notifier] = // this holds the DeviceBuilder::NotifyCallback callable
      builder->addObservable(BuildInfo{"observable", "Observable"},
          DataType::String,
          &readCallback,
          &enableObservationCallback);

  builder->addCallable(
      BuildInfo{"executable", "Simple fire and forget executable method"},
      &executeCallback);

  builder->addCallable(
      BuildInfo{"asyncExecutable", "Asynchronous executable method"},
      DataType::String,
      &executeCallback,
      &asyncExecuteCallback,
      &cancelAsyncExecuteCallback);

  auto group_id = builder->addGroup(BuildInfo{"group", "Group element"});
  builder->addReadable(group_id,
      BuildInfo{"sub_readable", "A readable element within a group"},
      DataType::Boolean,
      []() {
        // Your read operation
        return true;
      });

  auto subgroup_id = builder->addGroup(
      group_id, BuildInfo{"sub_group", "A group within another group"});
  builder->addWritable(subgroup_id,
      BuildInfo{"sub_sub_writable", "A writable element within a subgroup"},
      DataType::Boolean,
      [](const DataVariant&) {
        // Your write operation
      });

  return builder->result();
}
