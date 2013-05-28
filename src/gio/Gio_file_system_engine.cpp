#include "Gio_file_system_engine.h"
#include "Mount_manager.h"
#include "qt_gtk.h"
#include "Directory.h"
#include <QUrl>

Gio_file_system_engine::Gio_file_system_engine(Mount_manager *m)
  : mount_manager(m)
{
  trash_files_path = QDir::homePath() + "/.local/share/Trash/files";
  if (!QDir(trash_files_path).exists()) {
    qDebug() << "Warning: trash files path does not exist: " << trash_files_path;
  }
  //connect(m, SIGNAL(mounts_changed(QList<Gio_mount>)), this, SLOT(mounts_changed(QList<Gio_mount>)));
  //mounts = mount_manager->get_mounts();
}

File_system_engine::Iterator *Gio_file_system_engine::list(const QString &uri) {
  if (real_engine.is_responsible_for(uri)) return real_engine.list(uri);
  bool use_native = false;
  if (uri.startsWith("trash:") || uri.startsWith("network:")) {
    use_native = true;
  }
  {
    QStringList parts = uri.split("/", QString::SkipEmptyParts);
    if (parts.count() == 2 && parts[0] == "smb:") {
      use_native = true; // smb shares list
    }
  }

  if (use_native) {
    GFile* file = g_file_new_for_uri(uri.toLocal8Bit());
    GError* error = 0;
    GFileEnumerator* enumerator = g_file_enumerate_children(file, "standard::*,owner::user", G_FILE_QUERY_INFO_NONE, 0, &error);
    if (!enumerator) {
      QString message = QString::fromLocal8Bit(error->message);
      qDebug() << "error: " << message;
      g_error_free(error);
      //todo: error message reporting
      throw File_system_engine::Exception(directory_list_failed, gio_error, uri);
    }
    Gio_native_fs_iterator* i = new Gio_native_fs_iterator();
    i->file = file;
    i->enumerator = enumerator;
    i->uri_prefix = uri.endsWith("/")? uri: (uri + "/");
    return i;
  } else {
    QString r_uri = get_real_file_name(uri);
    if (r_uri.isEmpty()) {
      throw Exception(directory_list_failed, not_found, uri);
    }
    //todo: convert real filenames in error messages to virtual uris
    Real_file_system_engine::Real_fs_iterator* real_iterator = dynamic_cast<Real_file_system_engine::Real_fs_iterator*>(real_engine.list(r_uri));
    Gio_fs_iterator* i = new Gio_fs_iterator();
    i->iterator = real_iterator;
    i->uri_prefix = uri.endsWith("/")? uri: (uri + "/");
    return i;
  }
}

File_system_engine::Operation *Gio_file_system_engine::copy(const QString &source, const QString &destination, bool append_mode) {
  QString r_uri1 = get_real_file_name(source);
  if (r_uri1.isEmpty()) {
    throw Exception(copy_failed, not_found, r_uri1);
  }
  QString r_uri2 = get_real_file_name(destination);
  if (r_uri2.isEmpty()) {
    throw Exception(copy_failed, not_found, r_uri2);
  }
  return real_engine.copy(r_uri1, r_uri2, append_mode);
}

File_system_engine::Operation *Gio_file_system_engine::move(const QString &source, const QString &destination) {
  foreach(Gio_mount mount, mount_manager->get_mounts()) {
    if (!mount.uri.isEmpty() && source.startsWith(mount.uri) && destination.startsWith(mount.uri)) {
      //both files in the same mount
      QString r_uri1 = mount.path + "/" + source.mid(mount.uri.length());
      QString r_uri2 = mount.path + "/" + destination.mid(mount.uri.length());
      return real_engine.move(r_uri1, r_uri2, Real_file_system_engine::move_mode_system);
    }
  }
  QString r_uri1 = get_real_file_name(source);
  if (r_uri1.isEmpty()) {
    throw Exception(copy_failed, not_found, r_uri1);
  }
  QString r_uri2 = get_real_file_name(destination);
  if (r_uri2.isEmpty()) {
    throw Exception(copy_failed, not_found, r_uri2);
  }
  return real_engine.move(r_uri1, r_uri2, Real_file_system_engine::move_mode_copy);
}

void Gio_file_system_engine::remove(const QString &uri) {
  QString r_uri = get_real_file_name(uri);
  if (r_uri.isEmpty()) {
    throw Exception(directory_list_failed, not_found, uri);
  }
  real_engine.remove(r_uri);
}

void Gio_file_system_engine::make_directory(const QString &uri) {
  QString r_uri = get_real_file_name(uri);
  if (r_uri.isEmpty()) {
    throw Exception(directory_list_failed, not_found, uri);
  }
  real_engine.make_directory(r_uri);
}

//void Gio_file_system_engine::mounts_changed(QList<Gio_mount> new_mounts) {
//  mounts = new_mounts;
//}

QString Gio_file_system_engine::get_real_file_name(const QString &uri) {
  if (uri.startsWith("/")) return uri;
  if (uri.startsWith("trash:")) {
    QString headless_path = uri.mid(6);
    while(headless_path.startsWith("/")) {
      headless_path = headless_path.mid(1);
    }
    return trash_files_path + "/" + headless_path;
  }
  foreach(Gio_mount mount, mount_manager->get_mounts()) {
    if (!mount.uri.isEmpty() && uri.startsWith(mount.uri)) {
      return mount.path + "/" + uri.mid(mount.uri.length());
    }
  }
  return QString();
}

bool Gio_file_system_engine::is_responsible_for(const QString &uri) {
  if (uri.startsWith("/")) return false;
  if (get_real_file_name(uri).isEmpty()) return false;
  return true;
}

bool Gio_file_system_engine::is_file(const QString &uri) {
  QString r_uri = get_real_file_name(uri);
  if (r_uri.isEmpty()) {
    throw Exception(directory_list_failed, not_found, uri);
  }
  return real_engine.is_file(r_uri);
}

bool Gio_file_system_engine::is_directory(const QString &uri) {
  QString r_uri = get_real_file_name(uri);
  if (r_uri.isEmpty()) {
    throw Exception(directory_list_failed, not_found, uri);
  }
  return real_engine.is_directory(r_uri);
}

bool Gio_file_system_engine::Gio_fs_iterator::has_next() {
  return iterator->has_next();
}

File_info Gio_file_system_engine::Gio_fs_iterator::get_next_internal() {
  File_info i = iterator->get_next();
  i.uri = uri_prefix + QFileInfo(i.uri).fileName();
  return i;
}

Gio_file_system_engine::Gio_fs_iterator::~Gio_fs_iterator() {
  delete iterator;
}


bool Gio_file_system_engine::Gio_native_fs_iterator::has_next() {
  if (cached_item) { return true; }
  GError* e = 0;
  cached_item = g_file_enumerator_next_file(enumerator, 0, &e);
  if (cached_item) { return true; }
  if (!cached_item && !e) { return false; }
  qDebug() << "gio error" << e->message;
  throw File_system_engine::Exception(directory_list_failed, gio_error, uri_prefix);
}

File_info Gio_file_system_engine::Gio_native_fs_iterator::get_next_internal() {
  GFileInfo* info = cached_item;
  cached_item = 0;
  if (!info) {
    GError* e = 0;
    info = g_file_enumerator_next_file(enumerator, 0, &e);
    if (!info) {
      if (e) {
        qDebug() << "gio error" << e->message;
        throw File_system_engine::Exception(directory_list_failed, gio_error, uri_prefix);
      } else {
        qDebug() << "no items left!";
        throw File_system_engine::Exception(directory_list_failed, unknown_cause, uri_prefix);
      }
    }
  }
  GFileType type = g_file_info_get_file_type(info);
  File_info result;
  if (type == G_FILE_TYPE_MOUNTABLE) {
    char* uri = g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
    result.uri = Directory::canonize(QUrl::fromPercentEncoding(QByteArray(uri)));
    g_free(uri);
  } else {
    const char* name = g_file_info_get_name(info); //  g_file_info_get_attribute_byte_string(info, G_FILE_ATTRIBUTE_STANDARD_NAME);
    result.uri = uri_prefix + QString::fromLocal8Bit(name);
  }
  result.is_folder = type == G_FILE_TYPE_MOUNTABLE || type == G_FILE_TYPE_DIRECTORY;
  result.mime_type = QString::fromLocal8Bit(g_file_info_get_content_type(info));
  g_object_unref(info);
  return result;
}

Gio_file_system_engine::Gio_native_fs_iterator::~Gio_native_fs_iterator() {
  g_file_enumerator_close(enumerator, 0, 0);
  g_object_unref(enumerator);
  g_object_unref(file);
}
