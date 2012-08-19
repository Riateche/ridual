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
  error_type_exists,
  error_type_destination_inside_source
};
Q_DECLARE_METATYPE(Error_type)

enum Error_reaction {
  error_reaction_undefined,
  error_reaction_ask,
  error_reaction_retry,
  error_reaction_skip,
  error_reaction_abort,
  error_reaction_merge_dir,
  error_reaction_continue_writing,
  error_reaction_delete_existing,
  error_reaction_rename_existing,
  error_reaction_rename_new
};
Q_DECLARE_METATYPE(Error_reaction)

QList<Error_reaction> get_error_reactions(Error_type error_type,
                                     bool is_dir,
                                     bool is_interactive);


QString error_reaction_to_string(Error_reaction reaction);

#endif // ERROR_REACTION_H
