#ifndef CORE_H
#define CORE_H

#include <QObject>

class Main_window;
class Mount_manager;
class Bookmarks_file_parser;
class Directory_watcher;

class Core : public QObject {
  Q_OBJECT
public:
  explicit Core();
  ~Core();
  void init_gui();

  inline Main_window* get_main_window() { return main_window; }
  inline Bookmarks_file_parser* get_bookmarks() { return bookmarks; }
  inline Bookmarks_file_parser* get_user_dirs() { return user_dirs; }
  inline Mount_manager* get_mount_manager() {  return mount_manager; }
  inline Directory_watcher* get_directory_watcher() { return watcher; }

  inline bool get_sort_folders_before_files() { return sort_folders_before_files; }
  void set_sort_folders_before_files(bool v);

signals:
  void sort_folders_before_files_changed();
  
private slots:

private:
  Main_window* main_window;
  Bookmarks_file_parser* bookmarks;
  Bookmarks_file_parser* user_dirs;
  Mount_manager* mount_manager;
  Directory_watcher* watcher;
  QThread* watcher_thread;

  bool sort_folders_before_files;


};

#endif // CORE_H
