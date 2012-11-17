#include "Real_file_system_engine.h"
#include "utils.h"


Real_file_system_engine::Real_file_system_engine()
{
}

File_system_engine::Iterator *Real_file_system_engine::list(const QString &uri) {
  if (!QDir(uri).isReadable()) {
    //todo: find out cause
    throw Exception(directory_list_failed, unknown_cause);
  }
  Real_fs_iterator* i = new Real_fs_iterator();
  i->iterator = new QDirIterator(uri, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
  return i;
}

File_system_engine::Operation *Real_file_system_engine::copy(const QString &source, const QString &destination) {
  Copy_operation* o = new Copy_operation();
  o->file1.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  o->file2.exceptions(std::ofstream::failbit | std::ofstream::badbit | std::ofstream::eofbit);
  o->source_filename = source;
  try {
    o->file1.open(source.toLocal8Bit(), std::ifstream::in | std::ifstream::binary);
  } catch (std::ios_base::failure e) {
    delete o;
    throw Exception(source_file_read_failed, get_cause_from_errno());
  }

  if (QFile(destination).exists()) {
    throw Exception(destination_file_write_failed, already_exists);
  }

  try {
    o->file2.open(destination.toLocal8Bit(), std::ifstream::out | std::ifstream::trunc | std::ifstream::binary);
  } catch (std::ios_base::failure e) {
    delete o;
    throw Exception(destination_file_write_failed, get_cause_from_errno());
  }

  o->file_size = 0;
  try {
    o->file1.seekg(0, std::ios::end);
    o->file_size = o->file1.tellg();
    o->file1.seekg(0);
  } catch (std::ios_base::failure e) {
    delete o;
    throw Exception(source_file_read_failed, get_cause_from_errno());
  }
  return o;
}

File_system_engine::Operation *Real_file_system_engine::move(const QString &source, const QString &destination)
{
}

void Real_file_system_engine::remove(const QString &uri)
{
}

void Real_file_system_engine::make_directory(const QString &uri)
{
}

QString Real_file_system_engine::get_real_file_name(const QString &uri) {
  if (QFile::exists(uri)) return uri;
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
        throw Real_file_system_engine::Exception(Real_file_system_engine::file_remove_failed, Real_file_system_engine::unknown_cause);
      }
    }
    finished = true;
  }

  int count = 0;
  try {
    count = file1.readsome(copy_buffer, FS_ENGINE_BUFFER_SIZE);
  } catch (std::ios_base::failure e) {
    throw Real_file_system_engine::Exception(Real_file_system_engine::source_file_read_failed, Real_file_system_engine::get_cause_from_errno());
  }
  try {
    file2.write(copy_buffer, count);
  } catch (std::ios_base::failure e) {
    throw Real_file_system_engine::Exception(Real_file_system_engine::destination_file_write_failed, Real_file_system_engine::get_cause_from_errno());
  }
}
