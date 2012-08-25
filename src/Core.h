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

  inline Main_window* get_main_window() { return main_window; }
  inline Bookmarks_file_parser* get_bookmarks() { return bookmarks; }
  inline Bookmarks_file_parser* get_user_dirs() { return user_dirs; }
  inline Mount_manager* get_mount_manager() {  return mount_manager; }
  inline Directory_watcher* get_directory_watcher() { return watcher; }

signals:
  
private slots:

private:
  Main_window* main_window;
  Bookmarks_file_parser* bookmarks;
  Bookmarks_file_parser* user_dirs;
  Mount_manager* mount_manager;
  Directory_watcher* watcher;
  QThread* watcher_thread;


};

#endif // CORE_H
