#include "mkdir.h"
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <QObject>

//todo: remove this file

bool ridual_mkdir(QString path, QString& error_string) {
  bool r = mkdir(path.toLocal8Bit(), 0755) == 0;
  if (!r) {
    error_string = errno_to_string();
  }
  return r;
}

bool ridual_rmdir(QString path, QString& error_string) {
  bool r = rmdir(path.toLocal8Bit()) == 0;
  if (!r) {
    if (errno == EEXIST || errno == ENOTEMPTY) {
      error_string = QObject::tr("Directory is not empty");
    } else {
      error_string = errno_to_string();
    }
  }
  return r;
}

QString errno_to_string() {
  switch(errno) {
    case EACCES:
    case EPERM:
      return QObject::tr("Permission denied");
      break;
    case EEXIST:
      return QObject::tr("File or directory already exists");
      break;
    case EMLINK:
      return QObject::tr("The parent directory has too many links (entries)");
      break;
    case ENOSPC:
      return QObject::tr("File system is full");
      break;
    case EROFS:
      return QObject::tr("Read-only file system");
      break;
    case EBUSY:
      return QObject::tr("Directory is busy");
      break;
    case EINVAL:
      return QObject::tr("Invalid path");
      break;
    case ENOENT:
      return QObject::tr("File or directory not found");
      break;
    case ENOTDIR:
      return QObject::tr("Path is not directory");
      break;
    case ENAMETOOLONG:
      return QObject::tr("Path is too long");
      break;
    case ELOOP:
      return QObject::tr("Symbolic links loop");
      break;
    case EIO:
      return QObject::tr("Input/output error");
      break;
    default:
      return QString::fromLocal8Bit(strerror(errno));
  }
}
