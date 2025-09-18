
#ifndef __STAG_INFORMATION_MODEL_MOCKS_ELEMENT_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_ELEMENT_MOCK_HPP
#include "CallableMock.hpp"
#include "GroupMock.hpp"
#include "MetaInfoMock.hpp"
#include "ObservableMock.hpp"
#include "ReadableMock.hpp"
#include "WritableMock.hpp"

#include <Information_Model/Element.hpp>

namespace Information_Model::testing {

struct ElementMock : public virtual Element, public MetaInfoMock {
  ElementMock(const ElementFunction& function, const std::string& id,
      const std::optional<FullMetaInfo>& meta = std::nullopt);

  ~ElementMock() override = default;

  MOCK_METHOD(ElementType, type, (), (const final));
  MOCK_METHOD(ElementFunction, function, (), (const final));

private:
  void setOnCall() const;

  ElementType type_;
  ElementFunction function_;
};

using ElementMockPtr = std::shared_ptr<ElementMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_ELEMENT_MOCK_HPP