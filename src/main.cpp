#include <QtGui/QApplication>
#include "Core.h"

#include <QStyleFactory>

int main(int argc, char *argv[]) {

  QApplication a(argc, argv);

  Core core;
  core.init_gui();
  
  return a.exec();
}
