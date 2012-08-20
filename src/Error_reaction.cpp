#include "Error_reaction.h"
#include <QDebug>


QString error_reaction_to_string(Error_reaction reaction) {
  switch (reaction) {
  case error_reaction_ask: return QObject::tr("Ask");
  case error_reaction_retry: return QObject::tr("Retry");
  case error_reaction_skip: return QObject::tr("Skip");
  case error_reaction_abort: return QObject::tr("Abort");
  case error_reaction_merge_dir: return QObject::tr("Merge");
  case error_reaction_delete_existing: return QObject::tr("Delete existing");
  case error_reaction_rename_existing: return QObject::tr("Rename existing");
  case error_reaction_continue_writing: return QObject::tr("Continue writing");
  case error_reaction_rename_new: return QObject::tr("Rename new");
  default:
    qWarning("error_reaction_to_string failed");
    return QObject::tr("Unknown");
  }

}

QList<Error_reaction> get_error_reactions(Error_type error_type,
                                     bool is_dir,
                                     bool is_interactive) {
  if (error_type == no_error) {
    qWarning("Error_reaction::get_all: 'no_error' is not allowed");
    return QList<Error_reaction>();
  }
  QList<Error_reaction> r;
  r << error_reaction_skip;
  if (error_type == error_type_exists) {
    if (is_dir) {
      r << error_reaction_merge_dir;
    }
    r << error_reaction_delete_existing
      << error_reaction_rename_existing
      << error_reaction_rename_new;
    if (!is_dir) {
      r << error_reaction_continue_writing;
    }
  }
  r << error_reaction_abort;
  r.prepend(is_interactive? error_reaction_retry: error_reaction_ask);
  return r;
}
