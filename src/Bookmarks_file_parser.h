#ifndef BOOKMARKS_FILE_PARSER_H
#define BOOKMARKS_FILE_PARSER_H

#include <QObject>
#include <QFileSystemWatcher>
#include "File_info.h"
#include <QMutexLocker>

/*!
  This object is responsible for parsing of ~/.gtk-bookmarks and
  ~/.config/user-dirs.dirs files.

  These files contains GTK bookmarks and user directories (common places) of
  Gnome environment.

  This class is thread safe.

  */
class Bookmarks_file_parser : public QObject {
  Q_OBJECT
public:
  enum Format {
    format_gtk, //! Format of ~/.gtk-bookmarks file
    format_xdg  //! Format of ~/.config/user-dirs.dirs
  };

  /*!
   * Create an object that parses data from file_paths using specified format.
   */
  explicit Bookmarks_file_parser(const QStringList& file_paths, Format _format, QObject *parent = 0);

  //! Get all bookmarks
  inline File_info_list get_all() { QMutexLocker locker(&mutex); return list; }
  
signals:
  /*!
   * This signal is emitted when bookmarks data is updated, i.e. when the object
   * is constructed and when data source files are changed.
   */
  void changed();

private:
  QStringList file_paths;
  Format format;
  QFileSystemWatcher watcher;
  File_info_list list;
  QMutex mutex;

private slots:
  void read();

  
};

#endif // BOOKMARKS_FILE_PARSER_H
