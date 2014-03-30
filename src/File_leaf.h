#ifndef FILE_LEAF_H
#define FILE_LEAF_H

#include <QString>

/*
class File_leaf {
public:
  File_leaf(const QString& _name, bool is_folder, const QString& _parent_path);
  File_leaf(const QString& _name, bool is_folder, const File_leaf* _parent);

  inline QString get_relative_path() const { return get_path(false); }
  inline QString get_absolute_path() const { return get_path(true); }
  inline bool is_folder() const { return _is_folder; }


private:
  QString name;
  QString parent_path;
  const File_leaf* parent;
  bool _is_folder;

  QString get_path(bool absolute) const;
};
*/

#endif // FILE_LEAF_H
