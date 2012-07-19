#include "debug_output.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QTest>

::std::ostream& operator<<(::std::ostream& os, const QString& s) {
  return os << qPrintable(s);
}

bool wait_for_signal(QSignalSpy* spy, int max_time) {
  QElapsedTimer  t;
  t.start();
  while(spy->isEmpty()) {
    if (t.elapsed() > max_time) return false;
    QTest::qWait(50);
  }
  return true;
}
