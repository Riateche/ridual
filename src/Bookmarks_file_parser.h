#ifndef BOOKMARKS_FILE_PARSER_H
#define BOOKMARKS_FILE_PARSER_H

#include <QObject>
#include <QFileSystemWatcher>
#include "File_info.h"

class Bookmarks_file_parser : public QObject {
  Q_OBJECT
public:
  enum Format {
    format_gtk, //! Format of ~/.gtk-bookmarks file
    format_xdg  //! Format of ~/.config/user-dirs.dirs
  };

  explicit Bookmarks_file_parser(QString file_path, Format _format, QObject *parent = 0);
  inline File_info_list get_all() { return list; }
  
signals:
  void changed();

private:
  QString filename;
  Format format;
  QFileSystemWatcher watcher;
  File_info_list list;

private slots:
  void read();

  
};

#endif // BOOKMARKS_FILE_PARSER_H
