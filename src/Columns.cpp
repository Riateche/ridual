#include "Columns.h"
#include <QStringList>

const QMap<Column, QString>& Columns::get_all() {
  static QMap<Column, QString> r;
  if (r.isEmpty()) {
    r[column_name] = QObject::tr("Name");
    r[column_extension] = QObject::tr("Extension");
    r[column_full_name] = QObject::tr("Full name");
    r[column_parent_folder] = QObject::tr("Parent folder");
    r[column_full_path] = QObject::tr("Full path");
    r[column_uri]       = QObject::tr("Location");
    r[column_mime_type] = QObject::tr("MIME type");
    r[column_type_description] = QObject::tr("Type");
    r[column_date_modified] = QObject::tr("Date modified");
    r[column_date_accessed] = QObject::tr("Date accessed");
    r[column_date_created]  = QObject::tr("Date created");
    r[column_owner] = QObject::tr("Owner");
    r[column_owner_full_name] = QObject::tr("Owner name");
    r[column_group] = QObject::tr("Group");
    r[column_group_full_name] = QObject::tr("Group name");
    r[column_permissions] = QObject::tr("Permissions");
    r[column_octal_permissions] = QObject::tr("Octal permissions");
  }
  return r;
}

Columns Columns::get_default() {
  Columns r;
  r << column_full_name << column_mime_type << column_type_description << column_date_modified <<
       column_owner << column_octal_permissions;
  return r;
}

QVariant Columns::serialize() {
  QVariantList list;
  foreach(Column m, *this) {
    list << static_cast<int>(m);
  }
  return list;
}

Columns Columns::deserialize(QVariant data) {
  Columns r;
  foreach(QVariant v, data.toList()) {
    r << static_cast<Column>(v.toInt());
  }
  return r;
}

QString Columns::to_string() const {
  QStringList list;
  foreach(Column m, *this) {
    list << get_all()[m];
  }
  return QString("Columns(%1)").arg(list.join(", "));
}

