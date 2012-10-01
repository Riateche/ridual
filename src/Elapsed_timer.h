#ifndef ELAPSED_TIMER_H
#define ELAPSED_TIMER_H

#include <QTime>

// Qt 4.6 doesn't have QElapsedTimer yet

class Elapsed_timer {
public:
  Elapsed_timer();
  void restart();
  int elapsed();

private:
  QTime start_time;
};

#endif // ELAPSED_TIMER_H
