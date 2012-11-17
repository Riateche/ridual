/*
#include "gtest/gtest.h"
#include <QString>
#include <QStringList>
#include <QDebug>
#include <sys/stat.h>


TEST(stat_device, test1) {
  QStringList files;
  files << "/tmp/1";
  files << "/usr/share/readline/inputrc";
  files << "/home/ri/.inputrc";
  files << "/d/docs/1.lisp";
  files << "/d/docs/1.lisp";
  files << "/s/Games/run_gg.sh";
  foreach(QString s, files) {
    struct stat r;
    lstat(s.toLocal8Bit(), &r);
    qDebug() << s << r.st_dev;
  }
}
*/
