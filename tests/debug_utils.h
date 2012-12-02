#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include <QString>
#include <QSignalSpy>
#include "Columns.h"
#include "File_info.h"

void PrintTo(const QString& bar, ::std::ostream* os);
void PrintTo(const Columns& bar, ::std::ostream* os);

bool wait_for_signal(QSignalSpy* spy, int max_time = 3000);

bool uri_less_than(const File_info &v1, const File_info &v2);

QDir env_dir(const QString& test_name);

#endif // DEBUG_OUTPUT_H
