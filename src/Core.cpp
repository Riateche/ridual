#include "Core.h"
#include "Main_window.h"
#include <QMutex>
#include "Bookmarks_file_parser.h"
#include "Mount_manager.h"
#include "Directory_watcher.h"
#include <QDir>
#include <QThread>
#include "Actions_manager.h"
#include "gio/Gio_file_system_engine.h"

#include <QIcon>


void detect_theme_name() {
  const QString test_name = "inode-directory";
  QStringList themes;
  //good_themes << "Humanity";
  QString default_theme = QIcon::themeName();
  themes.prepend(default_theme);
  foreach(QString path, QIcon::themeSearchPaths()) {
    foreach(QString name, QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
      if (!themes.contains(name)) {
        themes << name;
      }
    }
  }
  qDebug() << "Available icon themes: " << themes;
  foreach(QString theme, themes) {
    QIcon::setThemeName(theme);
    if (QIcon::hasThemeIcon(test_name)) {
      qDebug() << "Using file icon theme: " << theme;
      return;
    }
  }
  QIcon::setThemeName(default_theme);
  qDebug() << "Failed to find any theme containing file icons";
}

Core::Core() {
  detect_theme_name();
  //qDebug() << QIcon::themeSearchPaths();
  //qDebug() << QIcon::themeName();
  //QIcon::setThemeName("Humanity");
  //QIcon::setThemeSearchPaths(QString::fromLocal8Bit(qgetenv("XDG_DATA_DIRS")).split(":"));
  //qDebug() << QIcon::themeSearchPaths();


  qRegisterMetaType<File_info_list>("File_info_list");
  qRegisterMetaType<Error_type::Enum>("Error_type::Enum");

  main_window = 0;

  bookmarks = new Bookmarks_file_parser(QDir::home().absoluteFilePath(".gtk-bookmarks"),
                                        Bookmarks_file_parser::format_gtk);
  user_dirs = new Bookmarks_file_parser(QDir::home().absoluteFilePath(".config/user-dirs.dirs"),
                                        Bookmarks_file_parser::format_xdg);
  mount_manager = new Mount_manager(this);
  fs_engine = new Gio_file_system_engine(mount_manager);
  watcher = new Directory_watcher(this);
  watcher_thread = new QThread(this);
  watcher_thread->start();
  watcher->moveToThread(watcher_thread);

  actions_manager = new Actions_manager(this);

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

File_system_engine *Core::get_file_system_engine() {
  return fs_engine;
}

