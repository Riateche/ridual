#ifndef GIO_FILE_SYSTEM_ENGINE_H
#define GIO_FILE_SYSTEM_ENGINE_H

#include "File_system_engine.h"
#include "Real_file_system_engine.h"
#include "Mount.h"

class Mount_manager;
typedef struct _GFile GFile;
typedef struct _GFileInfo GFileInfo;
typedef struct _GFileEnumerator GFileEnumerator;

// not thread-safe

class Gio_file_system_engine : public File_system_engine {
  Q_OBJECT
public:
  Gio_file_system_engine(Mount_manager* m);
  Iterator* list(const QString& uri);
  Operation* copy(const QString& source, const QString& destination, bool append_mode);
  Operation* move(const QString& source, const QString& destination);
  void remove(const QString& uri);
  void make_directory(const QString& uri);
  QString get_real_file_name(const QString& uri);
  bool is_responsible_for(const QString& uri);
  bool is_file(const QString& uri);
  bool is_directory(const QString& uri);

  static void move_to_trash(const QString& uri);
  static QString get_trash_original_path(const QString& uri);

private:
  Real_file_system_engine real_engine;
  Mount_manager* mount_manager;

  QString trash_files_path;
  //QList<Gio_mount> mounts;

  class Gio_fs_iterator: public Iterator {
  public:
    Real_file_system_engine::Iterator* iterator;
    QString uri_prefix;
    bool has_next();
    File_info get_next_internal();
    ~Gio_fs_iterator();


  };

  class Gio_native_fs_iterator: public Iterator {
  public:
    Gio_native_fs_iterator() : file(0), enumerator(0), cached_item(0) {}
    GFile* file;
    GFileEnumerator* enumerator;
    GFileInfo* cached_item;
    QString uri_prefix;

    bool has_next();
    File_info get_next_internal();
    ~Gio_native_fs_iterator();

  };

//private slots:
//  void mounts_changed(QList<Gio_mount> new_mounts);

};

#endif // GIO_FILE_SYSTEM_ENGINE_H
