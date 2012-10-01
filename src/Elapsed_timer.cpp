#include "Elapsed_timer.h"

Elapsed_timer::Elapsed_timer()
{
}

void Elapsed_timer::restart() {
  start_time = QTime::currentTime();
}

int Elapsed_timer::elapsed() {
  return start_time.msecsTo(QTime::currentTime());
}
