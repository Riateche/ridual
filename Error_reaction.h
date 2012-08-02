#ifndef ERROR_REACTION_H
#define ERROR_REACTION_H

#include <QList>

class Error_reaction {
public:
  enum Error_type {
    error_type_rw,
    error_type_dir_exists,
    error_type_file_exists
  };

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

  static QList<Error_reaction> get_all(Error_type error_type, bool is_interactive);

  Reaction reaction;
};

#endif // ERROR_REACTION_H
