#include <QApplication>
#include "Core.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  Core core(true);
  return a.exec();
  Q_UNUSED(core);
}
