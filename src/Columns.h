#ifndef COLUMNS_H
#define COLUMNS_H

#include <QList>
#include <QMap>
#include <QVariant>

namespace Column {

  /*! This enum represents a column of files list view.
    */
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

/*! This class provides several convinient methods for operating with lists of columns.   */
class Columns: public QList<Column::Enum> {
public:
  //! Get full list of supported columns
  static Columns get_all();

  //! Get default columns set
  static Columns get_default();
  QVariant serialize();
  static Columns deserialize(QVariant data);

  //! Column name used for displaying
  static QString name(Column::Enum column);

  //! String representation for debug purposes
  QString to_string() const;



};

#endif // COLUMNS_H
