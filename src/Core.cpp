#include "Core.h"
#include "Main_window.h"
#include <QMutex>
#include "Bookmarks_file_parser.h"
#include "Mount_manager.h"
#include "Directory_watcher.h"
#include <QDir>
#include <QThread>

Core::Core() {

  bookmarks = new Bookmarks_file_parser(QDir::home().absoluteFilePath(".gtk-bookmarks"),
                                        Bookmarks_file_parser::format_gtk);
  user_dirs = new Bookmarks_file_parser(QDir::home().absoluteFilePath(".config/user-dirs.dirs"),
                                        Bookmarks_file_parser::format_xdg);
  mount_manager = new Mount_manager(this);
  watcher = new Directory_watcher();
  watcher_thread = new QThread();
  watcher_thread->start();
  watcher->moveToThread(watcher_thread);



  main_window = new Main_window(this);
  main_window->show();
}

Core::~Core() {
  watcher_thread->quit();
  watcher_thread->wait();

}

