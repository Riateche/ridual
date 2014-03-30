#ifndef FILE_LIST_MODEL_H
#define FILE_LIST_MODEL_H

#include <QAbstractTableModel>
#include <QDir>
#include "File_info.h"
#include <QFileIconProvider>
#include "Columns.h"
#include <QCache>
#include "Core_ally.h"
#include <QSettings>

typedef QPair<File_info, QVariant> Sorting_pair;

/*!
 * Model used for displaying file lists in views. It's responsible for:
 *
 * - data formatting
 * - columns displaying
 * - sorting
 *
 * Currently it also uses current model index information to display
 * cursor in vertical header (though that's against the Model-View rules).
 *
 * This class is not thread safe. It's used only with Qt view widgets, so
 * it can be used from the GUI thread only.
 *
 */
class File_list_model : public QAbstractTableModel, public Core_ally {
  Q_OBJECT
public:
  File_list_model(Core* c);
  void set_files(File_info_list list);
  const File_info_list& get_files() const { return unsorted_list; }

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QModelIndex index_for_uri(QString uri);

  File_info get_file_info(const QModelIndex &index);

  static QString get_mime_description(QString mime_type);
  static QString format_octal_permissions(QFile::Permissions permissions);
  static QString format_string_permissions(QFile::Permissions permissions);

  void set_current_index(const QModelIndex& index);

  static const int sort_role = 100;


  void sort(int column, Qt::SortOrder order);

  inline int get_sort_column() { return sort_column; }
  inline Qt::SortOrder get_sort_order() { return sort_order; }

  inline Columns get_current_columns() { return current_columns; } //for testing

public slots:
  void set_columns(const Columns& new_columns);

private slots:
  void sort_folders_before_files_changed();

private:
  File_info_list list, unsorted_list;
  QModelIndex current_index;
  Columns columns;
  Columns current_columns;
  QSettings settings;


  static QHash<QString, QString> mime_descriptions;

  int sort_column;
  Qt::SortOrder sort_order;

  void update_current_columns();
  File_info_list sort_list(QList<Sorting_pair> &list, Qt::SortOrder order);
  void emit_row_changed(int row);
  inline void resort() { sort(sort_column, sort_order); }


};

#endif // FILE_LIST_MODEL_H
