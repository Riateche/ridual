#include "Columns.h"
#include <QStringList>

const QMap<Column::Enum, QString>& Columns::get_all() {
  static QMap<Column::Enum, QString> r;
  if (r.isEmpty()) {
    r[Column::name] = QObject::tr("Name");
    r[Column::extension] = QObject::tr("Extension");
    r[Column::full_name] = QObject::tr("Full name");
    r[Column::parent_folder] = QObject::tr("Parent folder");
    r[Column::full_path] = QObject::tr("Full path");
    r[Column::uri]       = QObject::tr("Location");
    r[Column::size]       = QObject::tr("Size");
    r[Column::mime_type] = QObject::tr("MIME type");
    r[Column::type_description] = QObject::tr("Type");
    r[Column::date_modified] = QObject::tr("Date modified");
    r[Column::date_accessed] = QObject::tr("Date accessed");
    r[Column::date_created]  = QObject::tr("Date created");
    r[Column::owner] = QObject::tr("Owner");
    r[Column::owner_full_name] = QObject::tr("Owner name");
    r[Column::group] = QObject::tr("Group");
    r[Column::group_full_name] = QObject::tr("Group name");
    r[Column::permissions] = QObject::tr("Permissions");
    r[Column::octal_permissions] = QObject::tr("Octal permissions");
  }
  return r;
}

Columns Columns::get_default() {
  Columns r;
  r << Column::full_name << Column::size << Column::type_description << Column::date_modified <<
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

QString Columns::to_string() const {
  QStringList list;
  foreach(Column::Enum m, *this) {
    list << get_all()[m];
  }
  return QString("Columns(%1)").arg(list.join(", "));
}

