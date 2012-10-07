#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include <QString>
#include <QSignalSpy>
#include "Columns.h"

void PrintTo(const QString& bar, ::std::ostream* os);
void PrintTo(const Columns& bar, ::std::ostream* os);

bool wait_for_signal(QSignalSpy* spy, int max_time = 3000);

#endif // DEBUG_OUTPUT_H
