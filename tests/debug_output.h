#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include <QString>
#include <QSignalSpy>
#include "Columns.h"

::std::ostream& operator<<(::std::ostream& os, const QString& s);
::std::ostream& operator<<(::std::ostream& os, const Columns& c);

bool wait_for_signal(QSignalSpy* spy, int max_time = 3000);

#endif // DEBUG_OUTPUT_H
