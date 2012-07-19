#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include <QString>
#include <QSignalSpy>

::std::ostream& operator<<(::std::ostream& os, const QString& s);

bool wait_for_signal(QSignalSpy* spy, int max_time = 3000);

#endif // DEBUG_OUTPUT_H
