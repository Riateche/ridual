#include "gtest/gtest.h"
#include "Columns.h"

TEST(Columns, serialize) {
  Columns c1;
  c1 << Column::name << Column::extension;
  Columns c2 = Columns::deserialize(c1.serialize());
  EXPECT_TRUE(c1 == c2);

  Columns c3;
  c1 << Column::name << Column::full_path;
  EXPECT_FALSE(c1 == c3);

}
