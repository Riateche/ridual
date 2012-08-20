#include "mkdir.h"
#include <sys/stat.h>
#include <errno.h>
#include <QObject>

bool ridual_mkdir(QString path, QString& error_string) {
  bool r = mkdir(path.toLocal8Bit(), 0755) == 0;
  if (!r) {
    switch(errno) {
      case EACCES:
        error_string = QObject::tr("Permission denied");
        break;
      case EEXIST:
        error_string = QObject::tr("File or directory already exists");
        break;
      case EMLINK:
        error_string = QObject::tr("The parent directory has too many links (entries)");
        break;
      case ENOSPC:
        error_string = QObject::tr("File system is full");
        break;
      case EROFS:
        error_string = QObject::tr("Read-only file system");
        break;
      default:
        error_string = QObject::tr("Unknown error");
    }
  }
  return r;
}
