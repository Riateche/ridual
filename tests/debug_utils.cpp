#include "debug_utils.h"
#include <QDebug>
#include "Elapsed_timer.h"
#include <QTest>
#include <QDir>
#include "gtest/gtest.h"

void PrintTo(const QString& bar, ::std::ostream* os) {
  *os << qPrintable(bar);
}


bool wait_for_signal(QSignalSpy* spy, int max_time) {
  Elapsed_timer t;
  t.restart();
  while(spy->isEmpty()) {
    if (t.elapsed() > max_time) return false;
    QTest::qWait(50);
  }
  return true;
}

void PrintTo(const Columns& bar, ::std::ostream* os) {
  PrintTo(bar.to_string(), os);
}

bool uri_less_than(const File_info &v1, const File_info &v2) {
  return v1.uri < v2.uri;
}


QDir env_dir(const QString& test_name) {
  QDir dir(TEST_ENV_PATH);
  EXPECT_TRUE(dir.cd(test_name));
  return dir;
}
