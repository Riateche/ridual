#ifndef FILE_LIST_MODEL_H
#define FILE_LIST_MODEL_H

#include <QAbstractTableModel>
#include <QDir>
#include "File_info.h"
#include <QFileIconProvider>
#include "Columns.h"

class File_list_model : public QAbstractTableModel {
public:
  File_list_model();
  void set_data(QList<File_info> list);
  void set_columns(const Columns& new_columns);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QModelIndex index_for_uri(QString uri);

  File_info info(const QModelIndex &index);

private:
  QList<File_info> list;
  Columns columns;

  static QString format_octal_permissions(QFile::Permissions permissions);

};

#endif // FILE_LIST_MODEL_H
