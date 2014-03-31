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
    octal_permissions,
    date_deleted,
    original_location
  };
}

/*! This class provides several convinient methods for operating with lists of columns.   */
class Columns: public QList<Column::Enum> {
public:
  /*! Get full list of columns available for regular folders
   * (special locations like "places", "trash:///" can have
   * columns that aren't listed here).
   */
  static Columns get_all_regular();

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
