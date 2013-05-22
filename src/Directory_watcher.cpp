#include "Directory_watcher.h"
#include <QDir>
#include <QDebug>
#include "Core.h"
#include "File_system_engine.h"


Directory_watcher::Directory_watcher(Core *c) :
  QObject(0)
, Core_ally(c)
{
  connect(&watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directory_changed(QString)));
  fs = core->get_file_system_engine();
}

void Directory_watcher::add(QString path) {
  //qDebug() << "Directory_watcher::add" << path;
  if (counter.contains(path)) {
    counter[path]++;
    //qDebug() << "already added";
  } else {
    //qDebug() << "adding";
    counter[path] = 1;
    QString real_path = fs->get_real_file_name(path);
    if (!real_path.isEmpty() && QDir(real_path).exists()) {
      real_paths[path] = real_path;
      watcher.addPath(real_path);
    }
  }
}

void Directory_watcher::remove(QString path) {
  //qDebug() << "Directory_watcher::remove" << path;
  counter[path]--;
  if (counter[path] == 0) {
    //qDebug() << "removing";
    counter.remove(path);
    real_paths.remove(path);
    watcher.removePath(path);
  }

}

void Directory_watcher::slot_directory_changed(QString real_path) {
  emit directory_changed(real_paths.key(real_path));
}
