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

  WritableMock(DataType type, const ReadCallback& read_cb,
      const WriteCallback& write_cb);

  ~WritableMock() override = default;

  void setWriteOnly(bool write_only) const;

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
   * NonReadable exception
   *
   * Same as @ref ReadableMock::updateReadCallback()
   *
   * @param read_cb
   */
  void updateReadCallback(const ReadCallback& read_cb);

  /**
   * @brief Changes the internal callback that is called for write() invocations
   * If the given callback is set to nullptr, forces the write() calls to throw
   * WriteCallbackUnavailable exception
   *
   * @param write_cb
   */
  void updateWriteCallback(const WriteCallback& write_cb);

  /**
   * @brief Updates both read() and write() invocation callbacks at the same
   * time Same as calling @ref updateReadCallback() and @ref
   * updateWriteCallback() one after another
   *
   * @param read_cb
   * @param write_cb
   */
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