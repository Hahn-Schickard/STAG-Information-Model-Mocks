#ifndef __STAG_INFORMATION_MODEL_MOCKS_TEST_RESOURCES_HPP
#define __STAG_INFORMATION_MODEL_MOCKS_TEST_RESOURCES_HPP

#include <Information_Model/DataVariant.hpp>
#include <Variant_Visitor/Visitor.hpp>

#include <functional>
#include <ostream>
#include <stdexcept>

namespace Information_Model::testing {
struct ReadableTestParam {
  using ReadCallback = std::function<DataVariant()>;

  explicit ReadableTestParam(const DataVariant& value)
      : value_(value), type_(toDataType(value)) {}

  explicit ReadableTestParam(DataType type) : type_(type) {
    if (auto value = setVariant(type_)) {
      value_ = value.value();
    } else {
      throw std::logic_error("Can not set " + toString(type) +
          " as a readable result expectation");
    }
  }

  ReadableTestParam(DataType type, const ReadCallback& callback)
      : value_(callback()), type_(type), read_cb_(callback) {}

  bool hasReadCallback() const { return read_cb_ != nullptr; }

  DataVariant readResult() const { return value_; }

  DataType dataType() const { return type_; }

  ReadCallback readCallback() const { return read_cb_; }

  // fix for https://github.com/google/googletest/issues/3805
  // NOLINTNEXTLINE(readability-identifier-naming)
  friend void PrintTo(const ReadableTestParam& param, std::ostream* os) {
    *os << "(value: " << toString(param.value_)
        << ", type: " << toString(param.type_) << ", callback: "
        << (param.read_cb_ == nullptr ? "nullptr"
                                      : std::to_string((size_t)&param.read_cb_))

        << ")";
  }

protected:
  DataVariant value_;
  DataType type_ = DataType::None;
  ReadCallback read_cb_ = nullptr;
};

inline DataVariant otherThan(const DataVariant& input) {
  return Variant_Visitor::match(
      input, // NOLINTBEGIN(readability-magic-numbers)
      [](bool) -> DataVariant {
        return std::vector<uint8_t>{0x00, 0x01, 0xAB};
      },
      [](intmax_t) -> DataVariant {
        return Timestamp{2025, 9, 10, 13, 01, 24, 32};
      },
      [](uintmax_t) -> DataVariant { return std::string("A new value"); },
      [](double) -> DataVariant { return uintmax_t{9789121}; },
      [](Timestamp) -> DataVariant { return intmax_t{-698872}; },
      [](const std::vector<uint8_t>&) -> DataVariant { return true; },
      [](const std::string&) -> DataVariant {
        return 20.3512;
      }); // NOLINTEND(readability-magic-numbers)
}

inline DataType otherThan(DataType input) {
  switch (input) {
  case DataType::Boolean: {
    return DataType::Opaque;
  }
  case DataType::Integer: {
    return DataType::Timestamp;
  }
  case DataType::Unsigned_Integer: {
    return DataType::String;
  }
  case DataType::Double: {
    return DataType::Integer;
  }
  case DataType::Timestamp: {
    return DataType::Unsigned_Integer;
  }
  case DataType::Opaque: {
    return DataType::Boolean;
  }
  case DataType::String: {
    return DataType::Double;
  }
  default: {
    return DataType::Unknown;
  }
  }
}

} // namespace Information_Model::testing
#endif //__STAG_INFORMATION_MODEL_MOCKS_TEST_RESOURCES_HPP