#include "File_list_model.h"
#include <QIcon>


File_list_model::File_list_model() {
}

void File_list_model::set_data(QList<File_info> p_list) {
  emit layoutAboutToBeChanged();
  list = p_list;
  emit layoutChanged();
}

int File_list_model::rowCount(const QModelIndex &parent) const {
  if (list.isEmpty()) return 1; //for "no files" caption
  return list.count();
  Q_UNUSED(parent);
}

QVariant File_list_model::data(const QModelIndex &index, int role) const {
  if (list.isEmpty() && index.row() == 0) {
    if (role == Qt::DisplayRole) {
      return tr("No files to display"); //todo: show count of hidden files
    }
  }

  if (index.row() < 0 || index.row() >= list.count()) return QVariant();
  const File_info& file_info = list.at(index.row());
  if (role == Qt::DisplayRole) {
    return file_info.caption;
  }
  if (role == Qt::DecorationRole) {
    return file_info.icon;
  }
  return QVariant();
}

Qt::ItemFlags File_list_model::flags(const QModelIndex &index) const {
  if (list.isEmpty() && index.row() == 0) {
    return 0; //not selectable, not enabled, not editable
  }
  if (index.row() < 0 || index.row() >= list.count()) return 0;
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;

}

QModelIndex File_list_model::index_for_uri(QString uri) {
  for(int i = 0; i < list.count(); i++) {
    if (list[i].uri == uri) return index(i);
  }
  return QModelIndex();
}

File_info File_list_model::info(const QModelIndex &index) {
  if (index.row() < 0 || index.row() >= list.count()) return File_info();
  return list[index.row()];
}
