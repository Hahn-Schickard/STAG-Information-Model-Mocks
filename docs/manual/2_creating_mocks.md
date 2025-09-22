# Creating Mocks {#mock_creation}

To test various **Technology Adapter** or **Data Consumer Adapter** implementation functionalities, some test data is needed. However, during development, such data is not available. To solve this problem, we provide standard mock implementations to all of the **Information Model** interfaces, which can be used in place of real data during the development stage. 

## Creating Device mocks

Devices are complete abstractions of modeled entities. Devices usually have multiple elements inside, some can even have nested elements within subgroups or sub-subgroups. Due to this, creating Device mocks can be a tedious and error prone process. To reduce this complexity we provide a couple of different ways when it comes to Device mock creation.

### Using the MockBuilder

Creating the Device mocks via the MockBuilder is the suggested way of creating complete Device mock instances, since it avoids the standard device creation pitfalls. Take note, that some methods in MockBuilder are not available in standard DeviceBuilder interface, and are there only to simplify mock building.

```cpp
#include <Information_Model_Mock/FakeExecutor.hpp>
#include <Information_Model_Mock/MockBuilder.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace Information_Model::testing {

auto builder = std::make_shared<MockBuilder>();
// allocate buffer string for future cast, NOT RECOMMENDED
string callable_id_1 = "";
// Create a default executor to avoid having to cast callable element into
// callable mock
auto executor = makeExecutor(DataType::String, ParameterTypes{},
    std::make_exception_ptr(std::runtime_error("Example error")),
    std::chrono::nanoseconds(100));
try {
  builder->setDeviceInfo(
      "unique_Device_id", BuildInfo{"device_name", "device description"});
  // Add an element to the root group
  auto parent_id = builder->addGroup(BuildInfo{"group_name"});
  { // add elements to a specific subgroup
    // Create a readable element that always returns the default boolean value
    auto readable_id_1 =
        builder->addReadable(parent_id, BuildInfo{}, DataType::Boolean);
    // Create a readable element that always returns a given double value
    auto readable_id_2 = builder->addReadable(parent_id, BuildInfo{}, 20.2);
    // Create a readable element that uses a generator function to return
    // generated value on read() call
    auto readable_id_3 = builder->addReadable(parent_id, BuildInfo{}, []() {
      // implement value generator here
      return true;
    });
    // Create a write-only writable element that accepts string values
    auto writable_id_1 =
        builder->addWritable(parent_id, BuildInfo{}, DataType::String);
    // Create a read-and writable element that accepts string values and returns
    // a given string message
    auto writable_id_2 =
        builder->addWritable(parent_id, BuildInfo{}, "Hello World");
    // Create a read-and writable or write-only element that uses a given
    // consumer and generator functions
    auto writable_id_3 = builder->addWritable(
        parent_id, BuildInfo{}, DataType::String,
        [](const DataVariant&) {
          // implement value consumer
        },
        []() {
          // implement value generator
          return "Hello World"
        });
    // Create an observable element that allow to observe Timestamp changes
    auto [observable_id, notifier] = builder->addObservable(
        parent_id, BuildInfo{}, DataType::Timestamp, [](bool) {
          // implement observation start/stop mechanism
        }); // don't forget to save the notifier Callable, you will need it to
            // dispatch notifications
    // Create a Callable element that returns no value and uses the internal
    // executor
    callable_id_1 = builder->addCallable(parent_id, BuildInfo{});
    // Create a Callable element that uses external callbacks
    auto callable_id_2 = builder->addCallable(
        parent_id, BuildInfo{},
        [](const Parameters&) {
          // implement executor here
        },
        [](const Parameters&) {
          // implement async executor here
          std::promise<DataVariant> promise;
          promise.set_exception(
              std::make_exception_ptr(std::runtime_error("Example error")));
          return ResultFuture(0, promise.get_future());
        },
        [](uintmax_t) {
          // implement canceler here
        });
    // Create a Callable element that uses an external executor
    auto callable_id_3 = builder->addCallable(BuildInfo{}, executor);
  }
} catch (const DeviceInfoNotSet&) {
  // this is thrown if setDeviceInfo() was not called
} catch (const DeviceBuildInProgress&) {
  // this is thrown if setDeviceInfo() was called multiple times before
  // builder->result()
} catch (const invalid_argument&) {
  // this is thrown if any of add* methods get and invalid argument
}

try {
  auto device = builder->result();
  // use your device
  // don't forget to get the callable objects and cast them to their mocks, so
  // you can start/stop responding to the calls
  auto callable = get<Callable>(device->element(callable_id_1)->function());
  auto callable_mock = dynamic_pointer_cast<CallableMock>(callable);
  callable_mock->start();
  // ... queue up requests or do other work
  callable_mock->stop(); // also don't forget to stop when you are finished
  // Or (preferably) use an external executor
  executor->start();
  // ... queue up requests or do other work
  executor->stop();
} catch (const DeviceInfoNotSet&) {
  // this is thrown if setDeviceInfo() was not called
} catch (const GroupEmpty&) {
  // this is thrown if any of groups added with addGroup method has no elements
  // within
} catch (const ElementNotFound&) {
  // this is thrown if device->element() does not contain an element with given
  // id
}
} // namespace Information_Model::testing
```

### Creating Device mock manually

We generally advice against creating Device mocks manually, since their creation is somewhat complex and error prone. However it is possible to create one manually as follows:

```cpp
#include <Information_Model_Mock/DeviceMock.hpp>
#include <Information_Model_Mock/ElementMock.hpp>

#include <functional>
#include <memory>
#include <string>

namespace Information_Model::testing {

// First, you need to create a device that will contain your elements
auto device = std::make_shared<DeviceMock>("unique_device_id",
    FullMetaInfo{"device_name", "A description of the device"});
// Then you need to create an element and assign it to the device
try {
  // first create the functional part of the element
  auto function = std::make_shared<ReadableMock>(DataType::Boolean);
  // then generate an id for said element
  auto id = device->generateID();
  // then create an Element mock using the function and id parts
  auto element = std::make_shared<ElementMock>(function, id);
  // Final add the newly built element to the device
  device->addElement(element);
  // if all went well, the device should have a new element with the generated
  // id
} catch (const std::invalid_argument&) {
  // if something went wrong, you would get an std::invalid_argument when adding
  // to the device

} catch (const std::logic_error&) {
  // or a std::logic_error exception when creating ElementMock instance or
  // adding it to the device
}

// You could simplify the above block as a one liner
try {
  device->addElement(std::make_shared<ElementMock>(
      std::make_shared<WritableMock>(DataType::Boolean), device->generateID());)
} catch (const std::exception&) {
  // you can also simplify exception handling to a single catch
}

// Adding subgroups is a bit different than normal elements
try {
  // When creating subgroup element you first need to generate the id
  auto id = device->generateID();
  // then use it to create your group functionality
  auto function = std::make_shared<GroupMock>(id);
  // then create it as an ElementMock
  auto element = std::make_shared<ElementMock>(function, id);
  // and finally ad it as before
  device->addElement(element);
} catch (const auto&) {
}
} // namespace Information_Model::testing
```

## Creating standalone Element mocks

Sometimes a fully fledged Device mock not necessary to test how specific interactions with specific Element types are done. In these cases it is possible to create Element instances that are not linked to any Device at all. 

### Creating a Readable mock
Creating a Readable mocks is very simple and can be done via one of the following constructors

```cpp
#include <Information_Model_Mock/ReadableMock.hpp>

#include <functional>
#include <memory>
#include <string>

namespace Information_Model::testing {
// Example function callback
DataVariant readCallback() {
  // implement your value generator
  return "Hello World"; // This is equivalent to DataType::String
}

// The simplest way of creating a mock is by specifying the modeled data type
// These mocks will always return the same default value for the specified data
// type
auto with_default_value_type = std::make_shared<ReadableMock>(DataType::String);

// Another quick way of creating a mock is by specifying the returned value
// These mocks will also always return the same value, but it is the one you
// specified
auto with_given_value = std::make_shared<ReadableMock>("Hello World");

// A more complex way of creating a mock is by providing a generator function
auto with_callback =
    std::make_shared<ReadableMock>(DataType::String, &readCallback);

// You can also use lambdas to generate values
auto with_lambda = std::make_shared<ReadableMock>(
    DataType::String, []() { return "Hello World"; });

// Example Class with read ability
struct Reader {
  DataVariant read() { return result_; }

  static DataVariant static_read() { return "Hello World"; }

private:
  DataVariant result_;
};

// Create an instance of your object
auto reader_object = std::make_shared<Reader>();

// Custom objects are also supported
auto with_object_method = std::make_shared<ReadableMock>(
    DataType::String, bind(&Reader::read, reader_object));

// Static class methods are also fine
auto with_static_method =
    std::make_shared<ReadableMock>(DataType::String, &Reader::static_read);
}
```

### Creating a Writable mock

Creating a Writable mock is really similar to creating Readable mocks, all of the Readable mock constructor signatures also apply for the Writable mocks, so we won't repeat them here. However Writable mock add a couple of new constructor signatures that allow to Mock read and write or write-only functionality.

```cpp
#include <Information_Model_Mock/WritableleMock.hpp>

#include <functional>
#include <memory>
#include <string>

namespace Information_Model::testing {
// Example function callback
void writeCallback(const DataVariant&) {
  // implement your value consumer
}

// The simplest way of creating a write only Writable is to specify the written
// data type and the write callback
auto write_only =
    std::make_shared<WritableMock>(DataType::String, &writeCallback);

// Example Class with read-write ability
struct Writer {
  DataVariant read() { return result_; }

  static DataVariant static_read() { return static_result_; }

  void write(const DataVariant& value) { result_ = value; }

  static void static_write(const DataVariant& value) { static_result_ = value; }

private:
  DataVariant result_;
  static DataVariant static_result_ = "Hello World";
};

// Create an instance of your object
auto writer_object = std::make_shared<Writer>();

// To create a fully read- and writable object you can use a custom class
auto with_object_method = std::make_shared<WritableMock>(DataType::String,
    bind(&Writer::write, writer_object, std::placeholders::_1),
    bind(&Writer::read, writer_object));

DataVariant buffer;

// You can also use lambdas as before
auto with_lambda_buffer = std::make_shared<WritableMock>(
    DataType::String, // modeled data type
    [&buffer](const DataVariant& value) { buffer = value }, // writer
    [&buffer]() { return buffer } // reader
);

// And as before, static class method work as well
auto with_lambda_buffer =
    std::make_shared<WritableMock>(DataType::String, // modeled data type
        &Writer::static_write, &Writer::static_read);

}
```

### Creating an Observable mock

Just like Writable mock creation, Observable mocks are very similar to Readable mocks and reuse all of the Readable constructor signatures. However, where it differs from Writable mock, is that it does not add any new signatures on it's own. The main difference between Observable and Readable mocks, is that Observable mocks add new observation functionality but use the same creation process.

### Creating a Callable mock

Creating Callable mocks can be a bit tricky if you want to emulate asynchronous functionality. To simplify  we recommend using the default provided `Executor` class. However if you need to, you can also provide your own `execute`, `asyncCall` and `cancel` callbacks


```cpp
#include <Information_Model_Mock/CallableMock.hpp>

#include <functional>
#include <memory>
#include <string>

namespace Information_Model::testing {
void executeCallback(const Parameters&) {
  // implement your executor
}

ResultFuture asyncCallback(const Parameters&) {
  // implement your async executor
  // you will need to provide thread synchronization and ResultFuture tracking
  // mechanisms on your own
}

void cancelCallback(uintmax_t) {
  // implement your canceler
  // you will need to provide thread synchronization and ResultFuture tracking
  // mechanisms on your own
}

// The simplest way of creating a mock is by specifying the modeled result data
// type. This will use the internal Executor to respond to your calls
auto default_executor = std::make_shared<CallableMock>(DataType::String,
    // You can specify which ParameterTypes this Callable should expect and
    // check on execute(), asyncCall() or call() invocations, however this is
    // optional and can be skipped
    ParameterTypes{
        // Optional parameter at position 0
        {0, ParameterType{DataType::Boolean, false}},
        // Mandatory parameter at position 1
        {1, ParameterType{DataType::Integer, true}},
    });

// Just like before you can use free function
auto external_callbacks = std::make_shared<CallableMock>(
    DataType::String, &executeCallback, &asyncCallback, &cancelCallback);

// Or you can use lambdas
auto in_place_lambdas = std::make_shared<CallableMock>(
    DataType::String,
    [](const Parameters&) {
      // executor implementation
    },
    [](const Parameters&) {
      std::promise<DataVariant> promised_result{};
      // Only for demonstration, you can fulfill the promise in a separate
      // process
      promised_result.set_exception(
          std::make_exception_ptr(std::runtime_error("Example exception")));
      return ResultFuture(0, promised_result.get_future());
    },
    [](uintmax_t) {
      // find the related ResultFuture and set CallCanceled exception
    });

// Or class methods or static class methods as shown previously
}
```

### Creating a Group mock

Groups are special kind of Element instances, that group other Elements (like the Readable, Writable, Observable, Callable or even other Groups) within themselves. For this reason their creation is a bit more difficult thant the over element instances. Creating a group is almost identical to creating a Device (since Device is a special kind of Group). Just use the manual Device mock creation example and replace the `device` variable with `group` and `DeviceMock` with `GroupMock`.
