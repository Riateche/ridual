#ifndef FILE_LIST_MODEL_H
#define FILE_LIST_MODEL_H

#include <QAbstractListModel>
#include <QFileInfo>
#include <QDir>

class File_list_model : public QAbstractListModel {
public:
  File_list_model();
  void set_data(QFileInfoList list);

  int rowCount(const QModelIndex &parent) const;

  QVariant data(const QModelIndex &index, int role) const;

private:
  QFileInfoList list;
};

#endif // FILE_LIST_MODEL_H
