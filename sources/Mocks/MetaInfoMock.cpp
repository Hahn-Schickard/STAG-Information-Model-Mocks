#include "MetaInfoMock.hpp"

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

MetaInfoMock::MetaInfoMock(const string& id, const optional<FullMetaInfo>& meta)
    : id_(id), meta_(meta) {
  ON_CALL(*this, id).WillByDefault(Return(id_));
  if (meta_) {
    ON_CALL(*this, name).WillByDefault(Return(meta_->name));
    ON_CALL(*this, description).WillByDefault(Return(meta_->description));
  }
}

} // namespace Information_Model::testing