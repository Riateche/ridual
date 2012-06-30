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
  if (list.isEmpty()) return 1;
  return columns.count();
}

QVariant File_list_model::headerData(int section, Qt::Orientation orientation, int role) const {
  if (list.isEmpty()) return QVariant();
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
        return file_info.full_name;
      }
      case column_name: {
        return file_info.name;
      }
      case column_extension: {
        return file_info.extension;
      }
      case column_parent_folder: {
        return file_info.parent_folder;
      }
      case column_full_path: {
        return file_info.file_path;
      }
      case column_owner: {
        return file_info.owner;
      }
      case column_group: {
        return file_info.group;
      }
      case column_octal_permissions: {
        return format_octal_permissions(file_info.permissions);
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

QString File_list_model::format_octal_permissions(QFile::Permissions permissions) {
  int r = 0;
  if (permissions & QFile::ReadOwner)   r += 0400;
  if (permissions & QFile::WriteOwner)  r += 0200;
  if (permissions & QFile::ExeOwner)    r += 0100;
  if (permissions & QFile::ReadGroup)   r += 0040;
  if (permissions & QFile::WriteGroup)  r += 0020;
  if (permissions & QFile::ExeGroup)    r += 0010;
  if (permissions & QFile::ReadOther)   r += 0004;
  if (permissions & QFile::WriteOther)  r += 0002;
  if (permissions & QFile::ExeOther)    r += 0001;
  return QString("%1").arg(r, 0, 8);
}
