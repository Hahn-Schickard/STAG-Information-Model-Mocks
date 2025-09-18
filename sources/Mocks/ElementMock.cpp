#include "ElementMock.hpp"

#include <Variant_Visitor/Visitor.hpp>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

ElementType getElementType(const ElementFunction& function) {
  return Variant_Visitor::match(
      function,
      [](const GroupPtr&) { return ElementType::Group; },
      [](const ReadablePtr&) { return ElementType::Readable; },
      [](const WritablePtr&) { return ElementType::Writable; },
      [](const ObservablePtr&) { return ElementType::Observable; },
      [](const CallablePtr&) { return ElementType::Callable; },
      [](auto) {
        throw logic_error(
            "Could not resolve ElementType based on ElementFunction value");
      });
}

ElementMock::ElementMock(const ElementFunction& function,
    const std::string& id,
    const std::optional<FullMetaInfo>& meta)
    : MetaInfoMock(id, meta), type_(getElementType(function)),
      function_(function) {
  setOnCall();
}

void ElementMock::setOnCall() const {
  ON_CALL(*this, type).WillByDefault(Return(type_));
  ON_CALL(*this, function).WillByDefault(Return(function_));
}

} // namespace Information_Model::testing
