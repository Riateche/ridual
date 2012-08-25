#ifndef COLUMNS_H
#define COLUMNS_H

#include <QList>
#include <QMap>
#include <QVariant>

namespace Column {
  enum Enum {
    name,
    extension,
    file_name,
    basename,
    parent_uri,
    full_path,
    uri,
    size,
    mime_type,
    type_description,
    date_modified,
    date_accessed,
    date_created,
    owner,
    group,
    permissions,
    octal_permissions
  };
}

class Columns: public QList<Column::Enum> {
public:
  static Columns get_all();
  static Columns get_default();
  QVariant serialize();
  static Columns deserialize(QVariant data);
  static QString name(Column::Enum column);



};

#endif // COLUMNS_H
