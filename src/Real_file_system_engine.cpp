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
  Real_fs_iterator* i = new Real_fs_iterator();
  i->iterator = new QDirIterator(uri, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
  return i;
}

File_system_engine::Operation *Real_file_system_engine::copy(const QString &source, const QString &destination) {
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

  if (QFile(destination).exists()) {
    throw Exception(copy_failed, already_exists, destination);
  }

  try {
    o->file2.open(destination.toLocal8Bit(), std::ifstream::out | std::ifstream::trunc | std::ifstream::binary);
  } catch (std::ios_base::failure e) {
    delete o;
    throw Exception(cant_open_file_for_write, get_cause_from_errno(), destination);
  }

  o->file_size = 0;
  try {
    o->file1.seekg(0, std::ios::end);
    o->file_size = o->file1.tellg();
    o->file1.seekg(0);
  } catch (std::ios_base::failure e) {
    delete o;
    throw Exception(file_not_seekable, get_cause_from_errno(), source);
  }
  return o;
}

File_system_engine::Operation *Real_file_system_engine::move(const QString &source, const QString &destination) {
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
  struct stat r;
  bool ok1, ok2;
  ok1 = lstat(source.toLocal8Bit(), &r) == 0;
  dev_t source_device = r.st_dev;
  QString destination_dir = QFileInfo(destination).absolutePath();
  ok2 = lstat(destination_dir.toLocal8Bit(), &r) == 0;
  dev_t destination_device = r.st_dev;
  if (ok1 && ok2 && source_device == destination_device) {
    /* If both files are on the same device, we must not copy
      them using buffer. Note that all GIO mountpoints considered
      as the same device, so we must not use this for GIO mountpoints.
      */
    if (!QFile::rename(source, destination)) {
      throw Exception(move_failed, unknown_cause, source, destination);
    }
    return new Done_operation();
  }
  Copy_operation* o = dynamic_cast<Copy_operation*>(copy(source, destination));
  if (!o) {
    qWarning("unexpected dynamic cast fail");
    throw Exception(move_failed, unknown_cause, source, destination);
  }
  o->must_delete_source = true;
  return o;
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
      if (c == already_exists) {
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
  if (uri.startsWith("/") && QFile::exists(uri)) return uri;
  return QString();
}

bool Real_file_system_engine::is_responsible_for(const QString &uri) {
  return uri.startsWith("/");
}

bool Real_file_system_engine::Real_fs_iterator::has_next() {
  return iterator->hasNext();
}

File_info Real_file_system_engine::Real_fs_iterator::get_next() {
  iterator->next();
  File_info item;
  item.uri = iterator->filePath();
  QFileInfo info(item.uri);
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
  item.mime_type = get_mime_type(item.uri);
  return item;
}


double Real_file_system_engine::Copy_operation::get_progress() {
  return 1.0 * file1.tellg() / file_size;
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
