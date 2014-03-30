#include "Bookmarks_file_parser.h"

#include <QDir>
#include <QUrl>
#include <QTextStream>
#include <QDebug>
#include "Directory.h"

Bookmarks_file_parser::Bookmarks_file_parser(const QStringList &_file_paths,
                                             Format _format,
                                             QObject *parent) :
  QObject(parent),
  file_paths(_file_paths),
  format(_format)
{
  bool found_any = false;
  foreach(QString file_path, file_paths) {
    if (QFile::exists(file_path)) {
      found_any = true;
      watcher.addPath(file_path);
    }
  }
  if (!found_any) {
    qDebug() << "bookmarks file not found; searched in " << file_paths;
    return;
  }
  connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(read()));
  read();
}

void Bookmarks_file_parser::read() {
  QMutexLocker locker(&mutex);
  list.clear();
  foreach(QString filename, file_paths) {
    QFile file(filename);
    if (!file.exists()) { continue; }
    if (!file.open(QFile::ReadOnly)) {
      qWarning("Failed to read bookmarks file %s", filename.toLocal8Bit().constData());
      return;
    }
    if (format == format_gtk) {
      QTextStream stream(&file);
      stream.setCodec("UTF-8");
      while(!stream.atEnd()) {
        QString s = stream.readLine().trimmed();
        if (s.isEmpty()) continue;
        File_info f;
        int i = s.indexOf(' ');
        if (i >= 0) {
          f.uri = QUrl::fromPercentEncoding(s.left(i).toLatin1());
          f.name = s.mid(i + 1);
        } else {
          f.uri = QUrl::fromPercentEncoding(s.toLatin1());
          QStringList parts = f.uri.split("/");
          if (parts.last().isEmpty()) {
            f.name = f.uri;
          } else {
            f.name = parts.last();
          }
        }
        f.uri = Directory::canonize(f.uri);
        if (f.uri.startsWith("file://")) {
          f.uri = f.uri.mid(7);
        }
        f.is_folder = true;
        list << f;
      }
    } else if (format == format_xdg) {
      QMap<QString, QString> known_dirs;
      // accordingly to xdg-user-dirs-0.14 source package,
      // the following dirs can be used:
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
            //here should be shell unescaping
            if (value.startsWith("$HOME/")) {
              value = QDir::home().absoluteFilePath(value.mid(6));
            }
            File_info f;
            f.uri = value;
            f.name = known_dirs[key];
            f.is_folder = true;
            list << f;
          }
        }
      }
    } else {
      qWarning("unknown format");
      return;
    }
    watcher.removePath(filename);
    watcher.addPath(filename);
  }
  locker.unlock();
  emit changed();
}


