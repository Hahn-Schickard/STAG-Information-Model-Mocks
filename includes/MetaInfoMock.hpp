#ifndef __STAG_INFORMATION_MODEL_MOCKS_META_INFO_MOCK_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_META_INFO_MOCK_HPP
#include <Information_Model/MetaInfo.hpp>
#include <gmock/gmock.h>

#include <optional>

namespace Information_Model::testing {

struct FullMetaInfo {
  std::string name;
  std::string description;
};

struct MetaInfoMock : public virtual MetaInfo {
  MetaInfoMock() = default;

  explicit MetaInfoMock(const std::string& id,
      const std::optional<FullMetaInfo>& meta = std::nullopt);

  ~MetaInfoMock() override = default;

  MOCK_METHOD(std::string, id, (), (const final));
  MOCK_METHOD(std::string, name, (), (const final));
  MOCK_METHOD(std::string, description, (), (const final));

private:
  std::string id_;
  std::optional<FullMetaInfo> meta_;
};

using MetaInfoMockPtr = std::shared_ptr<MetaInfoMock>;
} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_META_INFO_MOCK_HPP