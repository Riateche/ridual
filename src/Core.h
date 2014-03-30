#ifndef CORE_H
#define CORE_H

#include <QObject>

class Main_window;
class Mount_manager;
class Bookmarks_file_parser;
class Directory_watcher;
class Actions_manager;
class File_system_engine;

/*!
   This is the 'God object'. It creates objects of the most important core classes and
   provides pointers to them. It also manages some application settings.

   This class is thread safe (no sync mechanizms are needed).


  */
class Core : public QObject {
  Q_OBJECT
public:
  explicit Core(bool init_gui = false);
  ~Core();

  inline Main_window* get_main_window() { return main_window; }
  inline Bookmarks_file_parser* get_bookmarks() { return bookmarks; }
  inline Bookmarks_file_parser* get_user_dirs() { return user_dirs; }
  inline Mount_manager* get_mount_manager() {  return mount_manager; }
  inline Directory_watcher* get_directory_watcher() { return watcher; }
  inline Actions_manager* get_actions_manager() { return actions_manager; }

  File_system_engine* get_file_system_engine();

  void emit_settings_changed() { emit settings_changed(); }

signals:
  void settings_changed();

private:
  Main_window* main_window;
  Bookmarks_file_parser* bookmarks;
  Bookmarks_file_parser* user_dirs;
  Mount_manager* mount_manager;
  Directory_watcher* watcher;
  QThread* watcher_thread;
  Actions_manager* actions_manager;
  File_system_engine* fs_engine;

};

#endif // CORE_H
