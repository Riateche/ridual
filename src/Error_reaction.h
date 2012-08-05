#ifndef ERROR_REACTION_H
#define ERROR_REACTION_H

#include <QList>
#include <QVariant>

enum Error_type {
  no_error,
  error_type_not_found,
  error_type_read_failed,
  error_type_create_failed,
  error_type_write_failed,
  error_type_delete_failed,
  error_type_exists
};
Q_DECLARE_METATYPE(Error_type)

class Error_reaction {
public:

  enum Reaction {
    ask,
    retry,
    skip,
    abort,
    merge_dir,
    continue_writing,
    delete_existing,
    rename_existing,
    rename_new
  };

  Error_reaction(Reaction r); //implicit
  QString get_text();

  static QList<Error_reaction> get_all(Error_type error_type,
                                       bool is_dir,
                                       bool is_interactive);

  Reaction reaction;
};

#endif // ERROR_REACTION_H
