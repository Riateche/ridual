#include "Core.h"
#include "Main_window.h"
#include <QMutex>
#include "Bookmarks_file_parser.h"
#include "Mount_manager.h"
#include "Directory_watcher.h"
#include <QDir>
#include <QThread>

Core::Core() {
  qRegisterMetaType<File_info_list>("File_info_list");
  qRegisterMetaType<Error_type::Enum>("Error_type::Enum");

  main_window = 0;

  bookmarks = new Bookmarks_file_parser(QDir::home().absoluteFilePath(".gtk-bookmarks"),
                                        Bookmarks_file_parser::format_gtk);
  user_dirs = new Bookmarks_file_parser(QDir::home().absoluteFilePath(".config/user-dirs.dirs"),
                                        Bookmarks_file_parser::format_xdg);
  mount_manager = new Mount_manager(this);
  watcher = new Directory_watcher();
  watcher_thread = new QThread();
  watcher_thread->start();
  watcher->moveToThread(watcher_thread);

  QSettings settings;
  sort_folders_before_files = settings.value("sort_folders_before_files").toBool();


}

Core::~Core() {
  watcher_thread->quit();
  watcher_thread->wait();

}

void Core::init_gui() {
  main_window = new Main_window(this);
  main_window->init();
}

void Core::set_sort_folders_before_files(bool v) {
  if (v == sort_folders_before_files) return;
  sort_folders_before_files = v;
  QSettings s;
  s.setValue("sort_folders_before_files", v);
  emit sort_folders_before_files_changed();
}

