#include "ReadableMock.hpp"

namespace Information_Model::testing {
using namespace ::testing;

ReadableMock::ReadableMock(DataType type) { updateType(type); }

ReadableMock::ReadableMock(const DataVariant& value) { updateValue(value); }

ReadableMock::ReadableMock(DataType type, const ReadCallback& read_cb)
    : ReadableMock(type) {
  updateReadCallback(read_cb);
}

void ReadableMock::updateType(DataType type) {
  type_ = type;
  ON_CALL(*this, dataType).WillByDefault(Return(type_));
}

void ReadableMock::updateReadCallback(const ReadCallback& read_cb) {
  // NOLINTNEXTLINE(bugprone-assignment-in-if-condition)
  if (read_ = read_cb) { // we want to assign and check if it was unset
    ON_CALL(*this, read).WillByDefault(read_);
  } else {
    ON_CALL(*this, read).WillByDefault(Throw(ReadCallbackUnavailable()));
  }
}

void ReadableMock::updateValue(const DataVariant& value) {
  value_ = value;
  ON_CALL(*this, read).WillByDefault(Return(value_.value()));
  updateType(toDataType(value));
}
} // namespace Information_Model::testing
