#ifndef __STAG_INFORMATION_MODEL_MOCKS_WRITABLE_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_WRITABLE_MOCK_HPP
#include "ReadableMock.hpp"

#include <Information_Model/Writable.hpp>

namespace Information_Model::testing {

struct WritableMock : public Writable {
  using ReadCallback = ReadableMock::ReadCallback;
  using WriteCallback = std::function<void(const DataVariant&)>;

  WritableMock() = default;

  explicit WritableMock(DataType type);

  WritableMock(DataType type, const ReadCallback& read_cb);

  explicit WritableMock(const DataVariant& value);

  WritableMock(DataType type, const WriteCallback& write_cb);

  WritableMock(
      const DataVariant& value, const WriteCallback& write_cb, bool write_only);

  WritableMock(DataType type, const ReadCallback& read_cb,
      const WriteCallback& write_cb);

  ~WritableMock() override = default;

  void setWriteOnly(bool write_only) const;

  void updateType(DataType type);

  void updateValue(const DataVariant& value);

  void updateReadCallback(const ReadCallback& read_cb);

  void updateWriteCallback(const WriteCallback& write_cb);

  void updateCallbacks(
      const ReadCallback& read_cb, const WriteCallback& write_cb);

  MOCK_METHOD(DataType, dataType, (), (const final));
  MOCK_METHOD(DataVariant, read, (), (const final));
  MOCK_METHOD(bool, isWriteOnly, (), (const final));
  MOCK_METHOD(void, write, (const DataVariant&), (const final));

private:
  void setReadableCalls() const;

  WriteCallback write_;
  ReadableMockPtr readable_;
};

using WritableMockPtr = std::shared_ptr<WritableMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_WRITABLE_MOCK_HPP