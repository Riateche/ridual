#include "debug_output.h"
#include <QDebug>
#include "Elapsed_timer.h"
#include <QTest>


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

