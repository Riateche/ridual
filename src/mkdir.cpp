#include "mkdir.h"
#include <sys/stat.h>
#include <errno.h>
#include <QObject>

bool ridual_mkdir(QString path, QString& error_string) {
  bool r = mkdir(path.toLocal8Bit(), 0755) == 0;
  if (!r) {
    errno_to_string(error_string);
  }
  return r;
}

bool ridual_rmdir(QString path, QString& error_string) {
  bool r = rmdir(path.toLocal8Bit()) == 0;
  if (!r) {
    if (errno == EEXIST || errno == ENOTEMPTY) {
      error_string = QObject::tr("Directory is not empty");
    } else {
      errno_to_string(error_string);
    }
  }
  return r;
}

QString errno_to_string(QString& error_string) {
  switch(errno) {
    case EACCES:
    case EPERM:
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
    case EBUSY:
      error_string = QObject::tr("Directory is busy");
      break;
    case EINVAL:
      error_string = QObject::tr("Invalid path");
      break;
    case ENOENT:
      error_string = QObject::tr("File or directory not found");
      break;
    case ENOTDIR:
      error_string = QObject::tr("Path is not directory");
      break;
    case ENAMETOOLONG:
      error_string = QObject::tr("Path is too long");
      break;
    case ELOOP:
      error_string = QObject::tr("Symbolic links loop");
      break;
    case EIO:
      error_string = QObject::tr("Input/output error");
      break;
    default:
      error_string = QObject::tr("Unknown error");
  }
}
