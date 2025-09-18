#include "DeviceMock.hpp"

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

DeviceMock::DeviceMock(
    const string& base_id, const optional<FullMetaInfo>& meta)
    : MetaInfoMock(base_id, meta),
      group_(make_shared<NiceMock<GroupMock>>(base_id + ":")) {
  ON_CALL(*this, group).WillByDefault(Return(group_));
  ON_CALL(*this, size).WillByDefault(Invoke(group_.get(), &Group::size));
  ON_CALL(*this, element).WillByDefault(Invoke(group_.get(), &Group::element));
  ON_CALL(*this, visit).WillByDefault(Invoke(group_.get(), &Group::visit));
}

string DeviceMock::generateID() { return group_->generateID(); }

void DeviceMock::addElement(const ElementPtr& element) {
  group_->addElement(element);
}

} // namespace Information_Model::testing