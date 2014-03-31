#include "Real_file_system_engine.h"
#include "utils.h"
#include <sys/stat.h>
#include <QDebug>
#include <unistd.h>

Real_file_system_engine::Real_file_system_engine() {
}

File_system_engine::Iterator *Real_file_system_engine::list(const QString &uri) {
  if (!uri.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(directory_list_failed, invalid_path, uri);
  }
  if (!QDir(uri).isReadable()) {
    //todo: find out cause
    throw Exception(directory_list_failed, unknown_cause, uri);
  }
  return new Real_fs_iterator(new QDirIterator(uri, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags));
}

File_system_engine::Operation *Real_file_system_engine::copy(const QString &source, const QString &destination, bool append_mode) {
  if (!source.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(copy_failed, invalid_path, source);
  }
  if (!destination.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(copy_failed, invalid_path, destination);
  }
  if (!QFile(source).exists()) {
    throw Exception(copy_failed, not_found, source);
  }
  if (!append_mode) {
    if (QDir(destination).exists()) {
      throw Exception(copy_failed, directory_already_exists, source);
    }
    if (QFile(destination).exists()) {
      throw Exception(copy_failed, file_already_exists, source);
    }
  }

  Copy_operation* o = new Copy_operation();
  o->file1.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  o->file2.exceptions(std::ofstream::failbit | std::ofstream::badbit | std::ofstream::eofbit);
  o->source_filename = source;
  o->destination_filename = destination;
  try {
    o->file1.open(source.toLocal8Bit(), std::ifstream::in | std::ifstream::binary);
  } catch (std::ios_base::failure e) {
    delete o;
    throw Exception(cant_open_file_for_read, get_cause_from_errno(), source);
  }

  qint64 append_current_size = 0;
  std::ios_base::openmode mode = std::ifstream::out | std::ifstream::binary;
  if (append_mode) {
    std::ifstream f;
    f.open(destination.toLocal8Bit(), std::ios::binary | std::ios::ate);
    append_current_size = f.tellg();
    f.close();

    mode |= std::ifstream::app; //append
  } else {
    mode |= std::ifstream::trunc; //truncate
  }
  try {
    o->file2.open(destination.toLocal8Bit(), mode);
  } catch (std::ios_base::failure e) {
    delete o;
    throw Exception(cant_open_file_for_write, get_cause_from_errno(), destination);
  }

  o->file_size = 0;
  try {
    o->file1.seekg(0, std::ios::end);
    o->file_size = o->file1.tellg();
    o->file1.seekg(append_current_size);
  } catch (std::ios_base::failure e) {
    delete o;
    throw Exception(file_not_seekable, get_cause_from_errno(), source);
  }

  if (!QFile(destination).setPermissions(QFile(source).permissions())) {
    qDebug() << "Failed to copy permissions";
  }
  return o;
}

File_system_engine::Operation *Real_file_system_engine::move(const QString &source, const QString &destination) {
  return move(source, destination, move_mode_auto);
}

bool Real_file_system_engine::is_file(const QString &uri) {
  if (!uri.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(stat_failed, invalid_path, uri);
  }
  return QFile(uri).exists() && !(QDir(uri).exists());
}

bool Real_file_system_engine::is_directory(const QString &uri) {
  if (!uri.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(stat_failed, invalid_path, uri);
  }
  return QDir(uri).exists();
}

File_system_engine::Operation *Real_file_system_engine::move(const QString &source, const QString &destination, Move_mode move_mode) {
  if (!source.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(move_failed, invalid_path, source);
  }
  if (!destination.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(move_failed, invalid_path, destination);
  }
  if (!QFile(source).exists()) {
    throw Exception(move_failed, not_found, source);
  }
  if (QDir(destination).exists()) {
    throw Exception(copy_failed, directory_already_exists, source);
  }
  if (QFile(destination).exists()) {
    throw Exception(copy_failed, file_already_exists, source);
  }
  if (move_mode == move_mode_auto) {
    struct stat r;
    bool ok1, ok2;
    ok1 = lstat(source.toLocal8Bit(), &r) == 0;
    dev_t source_device = r.st_dev;
    QString destination_dir = QFileInfo(destination).absolutePath();
    ok2 = lstat(destination_dir.toLocal8Bit(), &r) == 0;
    dev_t destination_device = r.st_dev;
    if (ok1 && ok2 && source_device == destination_device) {
      move_mode = move_mode_system;
    } else {
      move_mode = move_mode_copy;
    }
  }
  qDebug() << "move mode: " << (move_mode == move_mode_system ? "system" : "copy");
  if (move_mode == move_mode_system) {
    /* If both files are on the same device, we must not copy
      them using buffer. Note that all GIO mountpoints considered
      as the same device, so we must not use this for GIO mountpoints.
      */
    if (!QFile::rename(source, destination)) {
      throw Exception(move_failed, unknown_cause, source, destination);
    }
    return new Done_operation();
  } else if (move_mode == move_mode_copy) {
    Copy_operation* o = dynamic_cast<Copy_operation*>(copy(source, destination, false));
    if (!o) {
      qWarning("unexpected dynamic cast fail");
      throw Exception(move_failed, unknown_cause, source, destination);
    }
    o->must_delete_source = true;
    return o;
  } else {
    qWarning("unexpected situation");
    throw Exception(move_failed, unknown_cause, source, destination);
  }
}

void Real_file_system_engine::remove(const QString &uri) {
  if (!uri.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(file_remove_failed, invalid_path, uri);
  }
  if (!QFile(uri).exists()) {
    throw Exception(file_remove_failed, not_found, uri);
  }
  if (QDir(uri).exists()) {
    bool r = rmdir(uri.toLocal8Bit()) == 0;
    if (!r) {
      error_cause c = get_cause_from_errno();
      if (c == file_already_exists) {
        c = directory_not_empty;
      }
      throw Exception(rmdir_failed, c, uri);
    }
    return;
  }
  if (!QFile(uri).remove()) {
    //todo: find out cause
    throw Exception(file_remove_failed, unknown_cause, uri);
  }
}

void Real_file_system_engine::make_directory(const QString &uri) {
  if (!uri.startsWith("/")) {
    qWarning("Real_file_system_engine supports only absolute paths");
    throw Exception(mkdir_failed, invalid_path, uri);
  }
  bool r = mkdir(uri.toLocal8Bit(), 0755) == 0;
  if (!r) {
    throw Exception(mkdir_failed, get_cause_from_errno(), uri);
  }
}

QString Real_file_system_engine::get_real_file_name(const QString &uri) {
  if (uri.startsWith("/")) {
    return uri;
  }
  return QString();
}

bool Real_file_system_engine::is_responsible_for(const QString &uri) {
  return uri.startsWith("/");
}

Real_file_system_engine::Real_fs_iterator::Real_fs_iterator(QDirIterator *i)
  : iterator(i)
{
}

bool Real_file_system_engine::Real_fs_iterator::has_next() {
  return iterator->hasNext();
}

File_info Real_file_system_engine::Real_fs_iterator::get_next_internal() {
  iterator->next();
  File_info item;
  //item.path = iterator->filePath();
  QFileInfo info(iterator->filePath());
  item.is_folder = info.isDir();
  if (item.is_file()) {
    item.file_size = info.size();
  } else {
    item.file_size = -1;
  }
  //todo: add flags to disable some fields retrieving
  item.owner = info.owner();
  item.group = info.group();
  item.permissions = info.permissions();
  item.date_accessed = info.lastRead();
  item.date_modified = info.lastModified();
  item.date_created = info.created();
  item.is_executable = item.is_file() && info.isExecutable();
  item.mime_type = get_mime_type(iterator->filePath());
  item.uri = iterator->filePath();
  return item;
}

double Real_file_system_engine::Copy_operation::get_progress() {
  double current_pos = 0;
  try {
    current_pos = file1.tellg();
  } catch (std::ios_base::failure e) {
    qDebug() << "Real_file_system_engine::Copy_operation::get_progress: exception: " << e.what();
  }
  return 1.0 * current_pos / file_size;
}

void Real_file_system_engine::Copy_operation::run_iteration() {
  if (finished) {
    qWarning("Real_file_system_engine::Copy_operation::run_iteration: called after finished");
    return;
  }
  if(file1.tellg() == file_size) {
    file1.close();
    file2.close();
    if (must_delete_source) {
      if (!QFile::remove(source_filename)) {
        //todo: find out cause
        throw Real_file_system_engine::Exception(Real_file_system_engine::file_remove_failed,
                                                 Real_file_system_engine::unknown_cause,
                                                 source_filename);
      }
    }
    finished = true;
  }

  int count = 0;
  try {
    count = file1.readsome(copy_buffer, FS_ENGINE_BUFFER_SIZE);
  } catch (std::ios_base::failure e) {
    throw Real_file_system_engine::Exception(Real_file_system_engine::file_read_failed,
                                             Real_file_system_engine::get_cause_from_errno(),
                                             source_filename);
  }
  try {
    file2.write(copy_buffer, count);
  } catch (std::ios_base::failure e) {
    throw Real_file_system_engine::Exception(Real_file_system_engine::file_write_failed,
                                             Real_file_system_engine::get_cause_from_errno(),
                                             destination_filename);
  }
}
