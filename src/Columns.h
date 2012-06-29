#ifndef COLUMNS_H
#define COLUMNS_H

#include <QList>
#include <QMap>
#include <QVariant>

enum Column {
  column_name,
  column_extension,
  column_full_name,
  column_parent_folder,
  column_full_path,
  column_mime_type,
  column_type_description,
  column_date_modified,
  column_date_accessed,
  column_owner,
  column_owner_full_name,
  column_group,
  column_group_full_name,
  column_permissions,
  column_octal_permissions
};

class Columns: public QList<Column> {
public:
  static QMap<Column, QString> get_all();
  static Columns get_default();
  QVariant serialize();
  static Columns deserialize(QVariant data);


};

#endif // COLUMNS_H
