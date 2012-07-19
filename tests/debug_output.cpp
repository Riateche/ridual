#include "debug_output.h"
#include <QDebug>

::std::ostream& operator<<(::std::ostream& os, const QString& s) {
  return os << qPrintable(s);
}
