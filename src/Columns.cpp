#include "Columns.h"
#include <QStringList>

Columns Columns::get_all() {
  Columns r;
  r    << Column::file_name;
  r    << Column::basename;
  r    << Column::extension;
  r    << Column::parent_uri;
  r    << Column::full_path;
  r    << Column::uri;
  r    << Column::size;
  r    << Column::mime_type;
  r    << Column::type_description;
  r    << Column::date_modified;
  r    << Column::date_accessed;
  r    << Column::date_created;
  r    << Column::owner;
  r    << Column::group;
  //r    << Column::permissions; //todo: implement
  r    << Column::octal_permissions;
  return r;
}

Columns Columns::get_default() {
  Columns r;
  r << Column::file_name << Column::size << Column::type_description << Column::date_modified <<
       Column::owner << Column::octal_permissions;
  return r;
}

QVariant Columns::serialize() {
  QVariantList list;
  foreach(Column::Enum m, *this) {
    list << static_cast<int>(m);
  }
  return list;
}

Columns Columns::deserialize(QVariant data) {
  Columns r;
  foreach(QVariant v, data.toList()) {
    r << static_cast<Column::Enum>(v.toInt());
  }
  return r;
}

QString Columns::name(Column::Enum r) {
  if (r == Column::name)                return QObject::tr("Name");
  if (r == Column::basename)            return QObject::tr("Basename");
  if (r == Column::extension)           return QObject::tr("Extension");
  if (r == Column::file_name)           return QObject::tr("Full name");
  if (r == Column::parent_uri)          return QObject::tr("Parent location");
  if (r == Column::full_path)           return QObject::tr("Full path");
  if (r == Column::uri)                 return QObject::tr("Location");
  if (r == Column::size)                return QObject::tr("Size");
  if (r == Column::mime_type)           return QObject::tr("MIME type");
  if (r == Column::type_description)    return QObject::tr("Type");
  if (r == Column::date_modified)       return QObject::tr("Date modified");
  if (r == Column::date_accessed)       return QObject::tr("Date accessed");
  if (r == Column::date_created)        return QObject::tr("Date created");
  if (r == Column::owner)               return QObject::tr("Owner");
  //if (r == Column::owner_full_name)     return QObject::tr("Owner name");
  if (r == Column::group)               return QObject::tr("Group");
  //if (r == Column::group_full_name)     return QObject::tr("Group name");
  if (r == Column::permissions)         return QObject::tr("Permissions");
  if (r == Column::octal_permissions)   return QObject::tr("Octal permissions");
  return QObject::tr("Invalid column");
}

QString Columns::to_string() const {
  QStringList list;
  foreach(Column::Enum c, *this) list << name(c);
  return QString("Columns(%1)").arg(list.join(", "));
}


