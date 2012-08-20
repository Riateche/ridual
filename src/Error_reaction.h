#ifndef ERROR_REACTION_H
#define ERROR_REACTION_H

#include <QList>
#include <QVariant>
#include "types.h"

namespace Error_reaction {
  enum Enum {
    undefined,
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
}
Q_DECLARE_METATYPE(Error_reaction::Enum)

QList<Error_reaction::Enum> get_error_reactions(Error_type::Enum error_type,
                                     bool is_dir,
                                     bool is_interactive);


QString error_reaction_to_string(Error_reaction::Enum reaction);

#endif // ERROR_REACTION_H
