#ifndef FILE_LIST_MODEL_H
#define FILE_LIST_MODEL_H

#include <QAbstractTableModel>
#include <QDir>
#include "File_info.h"
#include <QFileIconProvider>
#include "Columns.h"
#include <QCache>

class File_list_model : public QAbstractTableModel {
public:
  File_list_model();
  void set_data(File_info_list list);
  void set_columns(const Columns& new_columns);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QModelIndex index_for_uri(QString uri);

  File_info info(const QModelIndex &index);
  Columns get_current_columns() const;

  static QString get_mime_description(QString mime_type);

  void set_current_index(const QModelIndex& index);

private:
  File_info_list list;
  QModelIndex current_index;
  Columns columns;

  void emit_row_changed(int row);
  static QString format_octal_permissions(QFile::Permissions permissions);

  static QHash<QString, QString> mime_descriptions;

};

#endif // FILE_LIST_MODEL_H
