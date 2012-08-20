#ifndef COLUMNS_H
#define COLUMNS_H

#include <QList>
#include <QMap>
#include <QVariant>

namespace Column {
  enum Enum {
    name,
    extension,
    full_name,
    parent_folder,
    full_path,
    uri,
    mime_type,
    type_description,
    date_modified,
    date_accessed,
    date_created,
    owner,
    owner_full_name,
    group,
    group_full_name,
    permissions,
    octal_permissions
  };
}

class Columns: public QList<Column::Enum> {
public:
  static const QMap<Column::Enum, QString> &get_all();
  static Columns get_default();
  QVariant serialize();
  static Columns deserialize(QVariant data);

  QString to_string() const;


};

#endif // COLUMNS_H
