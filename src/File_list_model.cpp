#include "File_list_model.h"
#include <QIcon>
#include <QDebug>
#include <QFont>
#include "Directory.h"
#include "qt_gtk.h"
#include "Core.h"
#include "File_system_engine.h"
#include <qmath.h>

File_list_model::File_list_model(Core *c) : Core_ally(c) {
  connect(core, SIGNAL(settings_changed()),
          this, SLOT(sort_folders_before_files_changed()));
  sort_column = -1;
}

void File_list_model::set_files(File_info_list p_list) {
  emit layoutAboutToBeChanged();
  unsorted_list = p_list;
  list = p_list;
  update_current_columns();
  resort();
  emit layoutChanged();
}

void File_list_model::set_columns(const Columns &new_columns) {
  emit layoutAboutToBeChanged();
  columns = new_columns;
  update_current_columns();
  emit layoutChanged();
}

void File_list_model::sort_folders_before_files_changed() {
  resort();
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
    return Columns::name(column);
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
  if (column < 0 || column >= current_columns.count()) return QVariant();
  const File_info& file_info = list.at(row);
  if (role == Qt::FontRole &&
      current_columns[column] == Column::name) {
    QFont font;
    if (file_info.is_header_entry) {
      font.setBold(true);
    }
    return font;
  }
  if (role == Qt::TextAlignmentRole) {
    if (current_columns[column] == Column::size) {
      return static_cast<int>(Qt::AlignVCenter) | static_cast<int>(Qt::AlignRight);
    } else {
      return static_cast<int>(Qt::AlignVCenter) | static_cast<int>(Qt::AlignLeft);
    }
  }

  if (role == Qt::DisplayRole || role == Qt::EditRole || role == sort_role) {
    switch (current_columns[column]) {
      case Column::file_name: {
        return file_info.file_name();
      }
      case Column::name: {
        return file_info.name;
      }
      case Column::basename: {
        return file_info.basename();
      }
      case Column::extension: {
        return file_info.extension();
      }
      case Column::size: {
        if (role == sort_role) {
          return file_info.file_size;
        } else {
          if (file_info.file_size < 0) {
            return QVariant();
          } else {
            double base = 1024.0;
            QStringList units;
            units << tr("B") << tr("KiB") << tr("MiB") << tr("GiB") << tr("TiB");
            for(int i = 0; i < units.count(); i++) {
              if (file_info.file_size < qPow(base, i + 1) || i == units.count() - 1) {
                return QString("%1 %2").arg(file_info.file_size / qPow(base, i), 0, 'f', 1).arg(units[i]);
              }
            }
          }
        }
      }
      case Column::parent_uri: {
        return Directory::get_parent_uri(file_info.uri);
      }
      case Column::full_path: {
        return core->get_file_system_engine()->get_real_file_name(file_info.uri);
      }
      case Column::uri: {
        return file_info.uri;
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
      case Column::permissions: {
        return format_string_permissions(file_info.permissions);
      }
      case Column::date_modified: {
        if (role == sort_role) {
          return file_info.date_modified;
        } else {
          return file_info.date_modified.toString(Qt::SystemLocaleShortDate);
        }
      }
      case Column::date_created: {
        if (role == sort_role) {
          return file_info.date_created;
        } else {
          return file_info.date_created.toString(Qt::SystemLocaleShortDate);
        }
      }
      case Column::date_accessed: {
        if (role == sort_role) {
          return file_info.date_accessed;
        } else {
          return file_info.date_accessed.toString(Qt::SystemLocaleShortDate);
        }
      }
      case Column::date_deleted: {
        if (role == sort_role) {
          return file_info.date_deleted;
        } else {
          return file_info.date_deleted.toString(Qt::SystemLocaleShortDate);
        }
      }
      case Column::mime_type: {
        return file_info.mime_type;
      }
      case Column::original_location: {
        return file_info.original_location;
      }
      case Column::type_description: {
        return get_mime_description(file_info.mime_type);
      }
      default: {
        return "not implemented";
      }
    }

  }
  if (role == Qt::DecorationRole && column == 0) {
    return file_info.icon;
  }
  if (role == Qt::TextAlignmentRole) {
    if (current_columns[column] == Column::size) {
      return Qt::AlignRight;
    }
  }
  return QVariant();
}

Qt::ItemFlags File_list_model::flags(const QModelIndex &index) const {
  if (list.isEmpty() && index.row() == 0) {
    return 0; //not selectable, not enabled, not editable
  }
  if (index.row() < 0 || index.row() >= list.count() ||
      index.column() < 0 || index.column() >= current_columns.count()) { return 0; }
  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if (current_columns[index.column()] == Column::file_name) {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

QModelIndex File_list_model::index_for_uri(QString uri) {
  if (uri.isEmpty()) return QModelIndex();
  for(int i = 0; i < list.count(); i++) {
    if (list[i].uri == uri) return index(i, 0);
  }
  return QModelIndex();
}

bool File_list_model::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.row() < 0 || index.row() >= list.count() ||
      index.column() < 0 || index.column() >= current_columns.count()) { return false; }
  Column::Enum column = current_columns[index.column()];
  if (column != Column::file_name) { return false; }
  if (value.toString().isEmpty()) { return false; }
  Action_data action_data;
  action_data.type = Action_type::move;
  action_data.targets << list[index.row()];
  action_data.destination = Directory::get_parent_uri(action_data.targets[0].uri) + "/" + value.toString();
  action_data.destination_includes_filename = true;
  emit action_requested(action_data);
  return true;
}

File_info File_list_model::get_file_info(const QModelIndex &index) {
  if (index.row() < 0 || index.row() >= list.count()) return File_info();
  return list[index.row()];
}


QString File_list_model::get_mime_description(QString mime_type) {
  if (mime_type.isEmpty()) return tr("Unknown");
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


bool my_qvariant_sort(const Sorting_pair& v1, const Sorting_pair& v2) {
  QVariant::Type t1 = v1.second.type();
  QVariant::Type t2 = v2.second.type();
  if ( (t1 == QVariant::Int || t1 == QVariant::LongLong || t1 == QVariant::Invalid) &&
      (t2 == QVariant::Int || t2 == QVariant::LongLong || t2 == QVariant::Invalid)) {
    return v1.second.toLongLong() < v2.second.toLongLong();
  }
  if ( (t1 == QVariant::Date || t1 == QVariant::DateTime) &&
       (t2 == QVariant::Date || t2 == QVariant::DateTime) ) {
    return v1.second.toDateTime() < v2.second.toDateTime();
  }
  return v1.second.toString().toLower() < v2.second.toString().toLower();
 // return QLocale::system().toLower(v1.second.toString()) <
 //        QLocale::system().toLower(v2.second.toString());
}

void File_list_model::sort(int column, Qt::SortOrder order) {
  if (list.disable_sort) return;
  sort_column = column;
  sort_order = order;
  if (list.isEmpty()) return;
  emit layoutAboutToBeChanged();
  if (column == -1) {
    list = unsorted_list;
    if (settings.value("sort_folders_before_files", true).toBool()) {
      File_info_list folders, files;
      foreach(File_info fi, list) {
        (fi.is_folder ? folders:files) << fi;
      }
      list = folders;
      list.append(files);
    }
  } else {
    QList<Sorting_pair> r;
    for(int row = 0; row < list.count(); row++) {
      r << qMakePair(list[row], data(index(row, column), sort_role));
    }
    if (settings.value("sort_folders_before_files", true).toBool()) {
      QList<Sorting_pair> folders, files;
      foreach(Sorting_pair pair, r) {
        (pair.first.is_folder ? folders : files) << pair;
      }
      list = sort_list(folders, order);
      list.append(sort_list(files, order));
    } else {
      list = sort_list(r, order);
    }
  }
  emit layoutChanged();
}


File_info_list File_list_model::sort_list(QList<Sorting_pair>& some_list,
                                          Qt::SortOrder order) {
  qSort(some_list.begin(), some_list.end(), my_qvariant_sort);
  File_info_list result;
  foreach(Sorting_pair pair, some_list) {
    if (order == Qt::DescendingOrder) {
      result.prepend(pair.first);
    } else {
      result.append(pair.first);
    }
  }
  return result;
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
  qDebug() << "format" << static_cast<int>(permissions);
  int r = 0;
  if (permissions & QFile::ReadUser)   r += 0400;
  if (permissions & QFile::WriteUser)  r += 0200;
  if (permissions & QFile::ExeUser)    r += 0100;
  if (permissions & QFile::ReadGroup)   r += 0040;
  if (permissions & QFile::WriteGroup)  r += 0020;
  if (permissions & QFile::ExeGroup)    r += 0010;
  if (permissions & QFile::ReadOther)   r += 0004;
  if (permissions & QFile::WriteOther)  r += 0002;
  if (permissions & QFile::ExeOther)    r += 0001;
  return QString("%1").arg(r, 3, 8, QLatin1Char('0'));
  //return QString("%1").arg(static_cast<int>(permissions), 3, 8, QLatin1Char('0')).right(3);
}

QString File_list_model::format_string_permissions(QFile::Permissions permissions) {
  if (permissions == -1) return QString();
  QString s = "---------";
  if (permissions & QFile::ReadOwner)   s[0] = 'r';
  if (permissions & QFile::WriteOwner)  s[1] = 'w';
  if (permissions & QFile::ExeOwner)    s[2] = 'x';
  if (permissions & QFile::ReadGroup)   s[3] = 'r';
  if (permissions & QFile::WriteGroup)  s[4] = 'w';
  if (permissions & QFile::ExeGroup)    s[5] = 'x';
  if (permissions & QFile::ReadOther)   s[6] = 'r';
  if (permissions & QFile::WriteOther)  s[7] = 'w';
  if (permissions & QFile::ExeOther)    s[8] = 'x';
  return s;
}

QHash<QString, QString> File_list_model::mime_descriptions;
