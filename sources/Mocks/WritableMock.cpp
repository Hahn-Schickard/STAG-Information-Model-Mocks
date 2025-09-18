#include "WritableMock.hpp"

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

WritableMock::WritableMock(DataType type)
    : readable_(make_shared<NiceMock<ReadableMock>>(type)) {
  setReadableCalls();
  setWriteOnly(false);
}

WritableMock::WritableMock(DataType type, const ReadCallback& read_cb)
    : readable_(make_shared<NiceMock<ReadableMock>>(type, read_cb)) {
  setReadableCalls();
  setWriteOnly(!read_cb);
}

WritableMock::WritableMock(const DataVariant& value)
    : readable_(make_shared<NiceMock<ReadableMock>>(value)) {
  setReadableCalls();
  setWriteOnly(false);
}

WritableMock::WritableMock(DataType type, const WriteCallback& write_cb)
    : WritableMock(type) {
  setReadableCalls();
  readable_->updateType(type);
  updateWriteCallback(write_cb);
  setWriteOnly(true);
}

WritableMock::WritableMock(
    const DataVariant& value, const WriteCallback& write_cb, bool write_only)
    : WritableMock(value) {
  setReadableCalls();
  readable_->updateValue(value);
  updateWriteCallback(write_cb);
  setWriteOnly(write_only);
}

WritableMock::WritableMock(
    DataType type, const ReadCallback& read_cb, const WriteCallback& write_cb)
    : WritableMock(type, read_cb) {
  updateWriteCallback(write_cb);
}

void WritableMock::setReadableCalls() const {
  ON_CALL(*this, read)
      .WillByDefault(Invoke(readable_.get(), &ReadableMock::read));
  ON_CALL(*this, dataType)
      .WillByDefault(Invoke(readable_.get(), &ReadableMock::dataType));
}

void WritableMock::setWriteOnly(bool write_only) const {
  if (write_only) {
    ON_CALL(*this, read).WillByDefault(Throw(NonReadable()));
  }
  ON_CALL(*this, isWriteOnly).WillByDefault(Return(write_only));
}

void WritableMock::updateType(DataType type) { readable_->updateType(type); }

void WritableMock::updateValue(const DataVariant& value) {
  readable_->updateValue(value);
}

void WritableMock::updateReadCallback(const ReadCallback& read_cb) {
  if (read_cb) {
    readable_->updateReadCallback(read_cb);
    setReadableCalls();
  } else {
    setWriteOnly(true);
  }
}

void WritableMock::updateWriteCallback(const WriteCallback& write_cb) {
  // NOLINTNEXTLINE(bugprone-assignment-in-if-condition)
  if ((write_ = write_cb)) { // we want to assign and check if it was unset
    ON_CALL(*this, write).WillByDefault(write_);
  } else {
    ON_CALL(*this, write).WillByDefault(Throw(WriteCallbackUnavailable()));
  }
}

void WritableMock::updateCallbacks(
    const ReadCallback& read_cb, const WriteCallback& write_cb) {
  updateReadCallback(read_cb);
  updateWriteCallback(write_cb);
}
} // namespace Information_Model::testing
