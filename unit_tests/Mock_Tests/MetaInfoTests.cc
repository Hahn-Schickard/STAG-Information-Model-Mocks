#include "MetaInfoMock.hpp"

#include <gtest/gtest.h>

namespace Information_Model::testing {
using namespace std;
using namespace ::testing;

struct MetaInfoTestParam {
  string test_name;
  string id;
  string name;
  string description;

  explicit operator bool() const {
    return !id.empty() || !name.empty() || !description.empty();
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  friend void PrintTo(const MetaInfoTestParam& param, std::ostream* os) {
    *os << "(id: " << param.id << ", name:" << param.name
        << ", description:" << param.description << ")";
  }
};

struct MetaInfoTests : public TestWithParam<MetaInfoTestParam> {
  MetaInfoTests() : expected(GetParam()) {
    if (expected) {
      tested = make_shared<MetaInfoMock>(
          expected.id, FullMetaInfo{expected.name, expected.description});
    } else {
      tested = make_shared<MetaInfoMock>();
    }
  }

  MetaInfoTestParam expected;
  MetaInfoMockPtr tested;
};

TEST_P(MetaInfoTests, returnsId) {
  EXPECT_CALL(*tested, id).Times(Exactly(1));

  EXPECT_EQ(tested->id(), expected.id);
}

TEST_P(MetaInfoTests, returnsName) {
  EXPECT_CALL(*tested, name).Times(Exactly(1));

  EXPECT_EQ(tested->name(), expected.name);
}

TEST_P(MetaInfoTests, returnsDescription) {
  EXPECT_CALL(*tested, description).Times(Exactly(1));

  EXPECT_EQ(tested->description(), expected.description);
}

INSTANTIATE_TEST_SUITE_P(MetaInfoTestsValues,
    MetaInfoTests,
    Values( // clang-format off
        MetaInfoTestParam{
            "Empty",
            "",
            "",
            ""
        },
        MetaInfoTestParam{
            "ID_Only",
            "0", 
            "", 
            ""
        },
        MetaInfoTestParam{
            "Name_Only",
            "", 
            "no_id_is_bad", 
            ""
        },
        MetaInfoTestParam{
            "Description_Only",
            "", 
            "", 
            "Meta Info elements without ID are bad"
        },
        MetaInfoTestParam{
            "ID_And_Name",
            "1", 
            "fake_name", 
            ""
        },
        MetaInfoTestParam{
            "ID_And_Description",
            "2", 
            "", 
            "Meta Info elements without name but with ID are OK"
        },
        MetaInfoTestParam{
            "Simple_Fake",
            "3", 
            "fake", 
            "Fake Meta Info element"
        },
        MetaInfoTestParam{
            "Complex_ID_Fake",
            "id_0123456789!?=|[]{}()<>°^*+-~@#§$%&;:ßöäüµ`¸/\"\'\\", 
            "complex_id_fake", 
            "Fake Meta Info element with an ID containing special chars"
        },
        MetaInfoTestParam{
            "Complex_Name_Fake",
            "4", 
            "fake_0123456789!?=|[]{}()<>°^*+-~@#§$%&;:ßöäüµ`¸/\"\'\\", 
            "Fake Meta Info element with a name containing special chars"
        },
        MetaInfoTestParam{
            "Complex_Description_Fake",
            "5", 
            "complex_desc_fake", 
            "Fake Meta Info element with a description containing special chars\n"
            "_0123456789!?=|[]{}()<>°^*+-~@#§$%&;:ßöäüµ`¸/\"\'\\"
        },
        MetaInfoTestParam{
            "Long_ID",
            string(261,'1'), 
            "", 
            ""
        },
        MetaInfoTestParam{
            "Long_Name",
            "", 
            string(261,'a'), 
            ""
        },
        MetaInfoTestParam{
            "Long_Description",
            "", 
            "", 
            string(261,'d')
        },
        MetaInfoTestParam{
            "Very_Long_ID",
            string(32768,'1'), 
            "", 
            ""
        },
        MetaInfoTestParam{
            "Very_Long_Name",
            "", 
            string(32768,'a'), 
            ""
        },
        MetaInfoTestParam{
            "Very_Long_Description",
            "", 
            "", 
            string(32768,'d')
        }
    ), // clang-format on
    [](const TestParamInfo<MetaInfoTests::ParamType>& info) {
      return info.param.test_name;
    });
} // namespace Information_Model::testing