#ifndef FILE_LIST_MODEL_H
#define FILE_LIST_MODEL_H

#include <QAbstractListModel>
#include <QDir>
#include "File_info.h"
#include <QFileIconProvider>


class File_list_model : public QAbstractListModel {
public:
  File_list_model();
  void set_data(QList<File_info> list);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QModelIndex index_for_uri(QString uri);


  File_info info(const QModelIndex &index);

private:
  QList<File_info> list;

};

#endif // FILE_LIST_MODEL_H
