#include "gtest/gtest.h"
#include <QFile>

TEST(GTest, Trivial_assertion) {
  ASSERT_EQ(0, 0);
}

TEST(GTest, Env_path_exists) {
  ASSERT_TRUE(QFile::exists(TEST_ENV_PATH));
}
