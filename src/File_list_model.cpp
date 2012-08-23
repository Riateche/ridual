#include "File_list_model.h"
#include <QIcon>
#include <QDebug>
#include <QFont>

#include "qt_gtk.h"



File_list_model::File_list_model() {
}

void File_list_model::set_data(File_info_list p_list) {
  emit layoutAboutToBeChanged();
  list = p_list;
  update_current_columns();
  emit layoutChanged();
}

void File_list_model::set_columns(const Columns &new_columns) {
  emit layoutAboutToBeChanged();
  columns = new_columns;
  update_current_columns();
  emit layoutChanged();
}

int File_list_model::rowCount(const QModelIndex &parent) const {
  if (list.isEmpty()) return 1; //for "no files" caption
  return list.count();
  Q_UNUSED(parent);
}

int File_list_model::columnCount(const QModelIndex &parent) const {
  if (list.isEmpty()) return 1;
  return current_columns.count();
  Q_UNUSED(parent);
}

QVariant File_list_model::headerData(int section, Qt::Orientation orientation, int role) const {
  if (list.isEmpty()) return QVariant();
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    if (section < 0 || section >= current_columns.count()) return QVariant();
    Column::Enum column = current_columns.at(section);
    return Columns::get_all()[column];
  }
  if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
    //return QString("%1").arg(section + 1);
    return section == current_index.row()? ">": "";
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
    if (column < 0 || column >= current_columns.count()) return QVariant();
    switch (current_columns[column]) {
      case Column::full_name: {
        return file_info.full_name;
      }
      case Column::name: {
        return file_info.name;
      }
      case Column::extension: {
        return file_info.extension;
      }
      case Column::parent_folder: {
        return file_info.parent_folder;
      }
      case Column::full_path: {
        return file_info.full_path;
      }
      case Column::owner: {
        return file_info.owner;
      }
      case Column::group: {
        return file_info.group;
      }
      case Column::octal_permissions: {
        return format_octal_permissions(file_info.permissions);
      }
      case Column::date_modified: {
        return file_info.date_modified.toString(Qt::SystemLocaleShortDate);
      }
      case Column::date_created: {
        return file_info.date_created.toString(Qt::SystemLocaleShortDate);
      }
      case Column::date_accessed: {
        return file_info.date_accessed.toString(Qt::SystemLocaleShortDate);
      }
      case Column::mime_type: {
        return file_info.mime_type;
      }
      case Column::type_description: {
        return get_mime_description(file_info.mime_type);
      }
      case Column::uri: {
        return file_info.uri;
      }
      default: {
        return "not implemented";
      }
    }

  }
  if (role == Qt::DecorationRole && column == 0) {
    return file_info.icon;
  }
  /*if (role == Qt::FontRole) {
    QFont font;
    if (row == current_index.row()) {
      font.setBold(true);
      //if (column == 0) font.setUnderline(true);
    }
    return font;
  }*/
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

File_info File_list_model::get_file_info(const QModelIndex &index) {
  if (index.row() < 0 || index.row() >= list.count()) return File_info();
  return list[index.row()];
}


QString File_list_model::get_mime_description(QString mime_type) {
  if (mime_descriptions.contains(mime_type)) return mime_descriptions[mime_type];
  gchar* mime_description = g_content_type_get_description(mime_type.toLocal8Bit());
  QString r = QString::fromLocal8Bit(mime_description);
  g_free(mime_description);
  mime_descriptions.insert(mime_type, r);
  return r;
}

void File_list_model::set_current_index(const QModelIndex &new_index) {
  QModelIndex old = current_index;
  current_index = new_index;
  emit_row_changed(old.row());
  emit_row_changed(new_index.row());
}

void File_list_model::update_current_columns()  {
  if (!list.columns.isEmpty()) {
    current_columns = list.columns;
  } else {
    current_columns = columns;
  }

}

void File_list_model::emit_row_changed(int row) {
  if (row < 0) return;
  emit dataChanged(index(row, 0), index(row, columnCount()));
  emit headerDataChanged(Qt::Vertical, row, row);
}

QString File_list_model::format_octal_permissions(QFile::Permissions permissions) {
  if (permissions == -1) return QString();
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
  return QString("%1").arg(r, 3, 8, QLatin1Char('0'));
}

QHash<QString, QString> File_list_model::mime_descriptions;
