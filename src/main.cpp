#include <QtGui/QApplication>
#include "Main_window.h"
#include <QTextCodec>

#include <QStyleFactory>

int main(int argc, char *argv[])
{

  QApplication a(argc, argv);
  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
  //a.setStyle(QStyleFactory::create("Cleanlooks"));

  Main_window w;
  w.show();
  
  return a.exec();
}
