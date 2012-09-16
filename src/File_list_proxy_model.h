#ifndef FILE_LIST_PROXY_MODEL_H
#define FILE_LIST_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class File_list_proxy_model : public QSortFilterProxyModel {
  Q_OBJECT
public:
  File_list_proxy_model();
  bool lessThan ( const QModelIndex & left, const QModelIndex & right ) const;

private:

};

#endif // FILE_LIST_PROXY_MODEL_H
