#include "Gio_file_system_engine.h"
#include "Mount_manager.h"

Gio_file_system_engine::Gio_file_system_engine(Mount_manager *m)
  : mount_manager(m)
{
  connect(m, SIGNAL(mounts_changed(QList<Gio_mount>)), this, SLOT(mounts_changed(QList<Gio_mount>)));
  mounts = mount_manager->get_mounts();
}

File_system_engine::Iterator *Gio_file_system_engine::list(const QString &uri) {
  if (real_engine.is_responsible_for(uri)) return real_engine.list(uri);
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

File_system_engine::Operation *Gio_file_system_engine::copy(const QString &source, const QString &destination) {
  QString r_uri1 = get_real_file_name(source);
  if (r_uri1.isEmpty()) {
    throw Exception(copy_failed, not_found, r_uri1);
  }
  QString r_uri2 = get_real_file_name(destination);
  if (r_uri2.isEmpty()) {
    throw Exception(copy_failed, not_found, r_uri2);
  }
  return real_engine.copy(r_uri1, r_uri2);
}

File_system_engine::Operation *Gio_file_system_engine::move(const QString &source, const QString &destination) {
  foreach(Gio_mount mount, mounts) {
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

void Gio_file_system_engine::mounts_changed(QList<Gio_mount> new_mounts) {
  mounts = new_mounts;
}

QString Gio_file_system_engine::get_real_file_name(const QString &uri) {
  if (uri.startsWith("/")) return uri;
  foreach(Gio_mount mount, mounts) {
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

bool Gio_file_system_engine::Gio_fs_iterator::has_next() {
  return iterator->has_next();
}

File_info Gio_file_system_engine::Gio_fs_iterator::get_next_internal() {
  File_info i = iterator->get_next();
  i.uri = uri_prefix + QFileInfo(i.path).fileName();
  return i;
}

Gio_file_system_engine::Gio_fs_iterator::~Gio_fs_iterator() {
  delete iterator;
}
