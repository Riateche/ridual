#include "gtest/gtest.h"
#include <QFile>
#include <QDebug>

TEST(GTest, Trivial_assertion) {
  ASSERT_EQ(0, 0);
}

TEST(GTest, Env_path_exists) {
  #ifdef TEST_ENV_PATH
    qDebug() << "TEST_ENV_PATH =" << TEST_ENV_PATH;
    ASSERT_TRUE(QFile::exists(TEST_ENV_PATH));
  #else
    FAILURE("TEST_ENV_PATH is not defined");
  #endif
}
