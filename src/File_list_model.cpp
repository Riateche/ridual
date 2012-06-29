#include "File_list_model.h"
#include <QIcon>


File_list_model::File_list_model() {
}

void File_list_model::set_data(QList<File_info> p_list) {
  emit layoutAboutToBeChanged();
  list = p_list;
  emit layoutChanged();
}

void File_list_model::set_columns(const Columns &new_columns) {
  emit layoutAboutToBeChanged();
  columns = new_columns;
  emit layoutChanged();
}

int File_list_model::rowCount(const QModelIndex &parent) const {
  if (list.isEmpty()) return 1; //for "no files" caption
  return list.count();
  Q_UNUSED(parent);
}

int File_list_model::columnCount(const QModelIndex &parent) const {
  return columns.count();
}

QVariant File_list_model::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    if (section < 0 || section >= columns.count()) return QVariant();
    Column column = columns.at(section);
    return Columns::get_all()[column];
  }
  return QVariant();
}

QVariant File_list_model::data(const QModelIndex &index, int role) const {
  int row = index.row(), column = index.column();
  if (list.isEmpty() && row == 0) {
    if (role == Qt::DisplayRole) {
      return tr("No files to display"); //todo: show count of hidden files
    }
  }

  if (row < 0 || row >= list.count()) return QVariant();
  const File_info& file_info = list.at(row);
  if (role == Qt::DisplayRole) {
    if (column < 0 || column >= columns.count()) return QVariant();
    switch (columns[column]) {
      case column_full_name: {
        return file_info.caption;
      }
      default: {
        return "not implemented";
      }
    }

  }
  if (role == Qt::DecorationRole && column == 0) {
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
    if (list[i].uri == uri) return index(i, 0);
  }
  return QModelIndex();
}

File_info File_list_model::info(const QModelIndex &index) {
  if (index.row() < 0 || index.row() >= list.count()) return File_info();
  return list[index.row()];
}
