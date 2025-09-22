#ifndef __STAG_INFORMATION_MODEL_MOCKS_READABLE_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_READABLE_MOCK_HPP
#include <Information_Model/Readable.hpp>
#include <gmock/gmock.h>

#include <optional>

namespace Information_Model::testing {

struct ReadableMock : virtual public Readable {
  using ReadCallback = std::function<DataVariant()>;

  ReadableMock() = default;

  explicit ReadableMock(DataType type);

  explicit ReadableMock(const DataVariant& value);

  ReadableMock(DataType type, const ReadCallback& read_cb);

  ~ReadableMock() override = default;

  /**
   * @brief Change the modeled data type
   *
   * Sets the result of dataType() method calls
   *
   * @param type
   */
  void updateType(DataType type);

  /**
   * @brief Change the default read() and dataType() method results
   *
   * @attention Overrides the previous updateReadCallback() functionality
   *
   * @param value
   */
  void updateValue(const DataVariant& value);

  /**
   * @brief Change the internal callback that is used for read() invocations
   * If the given callback is set to nullptr, forces the read() calls to throw
   * ReadCallbackUnavailable exception
   *
   * @attention Overrides the previous updateValue() functionality
   *
   * @param read_cb
   */
  void updateReadCallback(const ReadCallback& read_cb);

  MOCK_METHOD(DataType, dataType, (), (const final));
  MOCK_METHOD(DataVariant, read, (), (const final));

private:
  DataType type_ = DataType::Boolean;
  std::optional<DataVariant> value_;
  ReadCallback read_;
};

using ReadableMockPtr = std::shared_ptr<ReadableMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_READABLE_MOCK_HPP