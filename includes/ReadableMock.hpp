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

  void updateType(DataType type);

  void updateValue(const DataVariant& value);

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