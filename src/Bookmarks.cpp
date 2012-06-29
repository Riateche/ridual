#include "Bookmarks.h"
#include <QDir>
#include <QUrl>
#include <QTextStream>
#include <QDebug>

Bookmarks::Bookmarks(QObject *parent) :
  QObject(parent)
{
  filename = QDir::home().absoluteFilePath(".gtk-bookmarks");
  filename_xdg = QDir::home().absoluteFilePath(".config/user-dirs.dirs");
  if (!QFile::exists(filename)) {
    qDebug() << "bookmarks file not found: " << filename;
    return;
  }
  if (!QFile::exists(filename_xdg)) {
    qDebug() << "xdg file not found: " << filename;
    return;
  }
  connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(read()));
  watcher.addPath(filename);
  read();

  connect(&watcher_xdg, SIGNAL(fileChanged(QString)), this, SLOT(read_xdg()));
  watcher_xdg.addPath(filename_xdg);
  read_xdg();
}

void Bookmarks::read() {
  list.clear();
  QFile file(filename);
  if (!file.open(QFile::ReadOnly)) {
    qWarning("Failed to read bookmarks file");
    return;
  }
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  while(!stream.atEnd()) {
    QString s = stream.readLine().trimmed();
    if (s.isEmpty()) continue;
    File_info f;
    int i = s.indexOf(' ');
    if (i >= 0) {
      f.uri = QUrl::fromPercentEncoding(s.left(i).toAscii());
      f.caption = s.mid(i + 1);
    } else {
      f.uri = QUrl::fromPercentEncoding(s.toAscii());
      if (f.uri.startsWith("file://")) {
        f.uri = f.uri.mid(7);
      }
      QStringList parts = f.uri.split("/");
      if (parts.last().isEmpty()) {
        f.caption = f.uri;
      } else {
        f.caption = parts.last();
      }
    }
    list << f;
  }
  emit changed();
  watcher.removePath(filename);
  watcher.addPath(filename);
}

void Bookmarks::read_xdg() {
  list_xdg.clear();
  QFile file(filename_xdg);
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "Failed to open ";
  }

  QMap<QString, QString> known_dirs;
  known_dirs["DESKTOP"]     = tr("Desktop");
  known_dirs["DOWNLOAD"]    = tr("Downloads");
  known_dirs["TEMPLATES"]   = tr("Templates");
  known_dirs["PUBLICSHARE"] = tr("Public");
  known_dirs["DOCUMENTS"]   = tr("Documents");
  known_dirs["MUSIC"]       = tr("Music");
  known_dirs["PICTURES"]    = tr("Pictures");
  known_dirs["VIDEOS"]      = tr("Videos");

  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  while(!stream.atEnd()) {
    QString line = stream.readLine().trimmed();
    foreach(QString key, known_dirs.keys()) {
      QString prefix = QString("XDG_%1_DIR=\"").arg(key);
      if (line.startsWith(prefix)) {
        QString value = line.mid(prefix.length(), line.length() - prefix.length() - 1);
        //todo: shell unescape
        if (value.startsWith("$HOME/")) {
          value = QDir::home().absoluteFilePath(value.mid(6));
        }
        File_info f;
        f.uri = value;
        f.caption = known_dirs[key];
        list_xdg << f;
      }
    }
  }

  emit changed();
  watcher_xdg.removePath(filename_xdg);
  watcher_xdg.addPath(filename_xdg);
}
