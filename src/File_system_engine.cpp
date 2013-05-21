#include "File_system_engine.h"
#include <QDebug>
#include <errno.h>

File_system_engine::File_system_engine() {

}

void File_system_engine::remove_recursively(const QString &uri) {
  if (is_directory(uri)) {
    Iterator* iterator = list(uri);
    while(iterator->has_next()) {
      remove_recursively(iterator->get_next().uri);
    }
    delete iterator;
  } else {
    remove(uri);
  }
}

File_system_engine::error_cause File_system_engine::get_cause_from_errno() {
  switch(errno) {
    case EACCES:
    case EPERM:
      return permission_denied;
      break;
    case EEXIST:
      return file_already_exists;
      break;
    case EMLINK:
      return too_many_entries;
      break;
    case ENOSPC:
      return filesystem_full;
      break;
    case EROFS:
      return readonly_filesystem;
      break;
    case EBUSY:
      return busy;
      break;
    case EINVAL:
      return invalid_path;
      break;
    case ENOENT:
      return not_found;
      break;
    case ENOTDIR:
      return not_directory;
      break;
    case ENAMETOOLONG:
      return path_too_long;
      break;
    case ELOOP:
      return symbolic_links_loop;
      break;
    case EIO:
      return io_error;
      break;
  case ENOTEMPTY:
      return directory_not_empty;
    default:
      return unknown_cause;
  }
}

File_system_engine::Exception::Exception() {

}

File_system_engine::Exception::Exception(File_system_engine::error_type p_type,
                                         error_cause p_cause,
                                         QString p_path1,
                                         QString p_path2)
: type(p_type)
, cause(p_cause)
, path1(p_path1)
, path2(p_path2)
{
  qDebug() << "Exception created: " << get_message();
}

QString File_system_engine::Exception::get_message() {
  QString type_string;
  switch(type) {
  case File_system_engine::unknown_error:
    type_string = QObject::tr("unknown error"); break;
  case File_system_engine::directory_list_failed:
    type_string = QObject::tr("failed to read directory contents"); break;
  case File_system_engine::cant_open_file_for_read:
    type_string = QObject::tr("cannot open file for reading"); break;
  case File_system_engine::cant_open_file_for_write:
    type_string = QObject::tr("cannot open file for writing"); break;
  case File_system_engine::copy_failed:
    type_string = QObject::tr("failed to copy file"); break;
  case File_system_engine::move_failed:
    type_string = QObject::tr("failed to move file"); break;
  case File_system_engine::file_not_seekable:
    type_string = QObject::tr("file is not seekable"); break;
  case File_system_engine::file_remove_failed:
    type_string = QObject::tr("failed to remove file"); break;
  case File_system_engine::file_read_failed:
    type_string = QObject::tr("failed to read from file"); break;
  case File_system_engine::file_write_failed:
    type_string = QObject::tr("failed to write to file"); break;
  case File_system_engine::stat_failed:
    type_string = QObject::tr("failed to get information about file"); break;
  case File_system_engine::mkdir_failed:
    type_string = QObject::tr("failed to make directory"); break;
  case File_system_engine::rmdir_failed:
    type_string = QObject::tr("failed to remove directory"); break;
  default:
    qWarning("File_system_engine::Exception::get_message: unhandled type");
    type_string = QObject::tr("unknown error");
  }

  QString cause_string;
  switch(cause) {
  case File_system_engine::unknown_cause:
    cause_string = QObject::tr("unknown"); break;
  case File_system_engine::not_found:
    cause_string = QObject::tr("not found"); break;
  case File_system_engine::permission_denied:
    cause_string = QObject::tr("permission denied"); break;
  case File_system_engine::file_already_exists:
    cause_string = QObject::tr("file already exists"); break;
  case File_system_engine::directory_already_exists:
    cause_string = QObject::tr("directory already exists"); break;
  case File_system_engine::too_many_entries:
    cause_string = QObject::tr("too many entries"); break;
  case File_system_engine::filesystem_full:
    cause_string = QObject::tr("filesystem is full"); break;
  case File_system_engine::readonly_filesystem:
    cause_string = QObject::tr("filesystem is readonly"); break;
  case File_system_engine::busy:
    cause_string = QObject::tr("device is busy"); break;
  case File_system_engine::invalid_path:
    cause_string = QObject::tr("invalid path specified"); break;
  case File_system_engine::not_directory:
    cause_string = QObject::tr("not a directory"); break;
  case File_system_engine::path_too_long:
    cause_string = QObject::tr("path is too long"); break;
  case File_system_engine::symbolic_links_loop:
    cause_string = QObject::tr("symbolic links loop"); break;
  case File_system_engine::io_error:
    cause_string = QObject::tr("input/output error"); break;
  case File_system_engine::directory_not_empty:
    cause_string = QObject::tr("directory is not empty"); break;
  default:
    qWarning("File_system_engine::Exception::get_message: unhandled type");
    cause_string = QObject::tr("unknown cause");
  }

  return QObject::tr("%1: '%3'%4. Reason: %2")
      .arg(type_string).arg(cause_string).arg(path1)
      .arg(path2.isEmpty() ? "" : QObject::tr(", '%1'").arg(path2));
}

File_info File_system_engine::Iterator::get_next() {
  qDebug() << "Iterator::get_next";
  if (!has_next()) {
    qWarning("File_system_engine::Iterator::get_next must not be called when has_next() == false");
    return File_info();
  }
  current = get_next_internal();
  return current;
}

File_system_engine::Simple_iterator::Simple_iterator(const File_info_list &p_list) {
  list = p_list;
  current = 0;
}

bool File_system_engine::Simple_iterator::has_next() {
  return current < list.count();
}

File_info File_system_engine::Simple_iterator::get_next_internal() {
  current++;
  return list[current - 1];
}

