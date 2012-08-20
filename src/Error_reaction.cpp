#include "Error_reaction.h"
#include <QDebug>


QString error_reaction_to_string(Error_reaction::Enum reaction) {
  switch (reaction) {
  case Error_reaction::ask: return QObject::tr("Ask");
  case Error_reaction::retry: return QObject::tr("Retry");
  case Error_reaction::skip: return QObject::tr("Skip");
  case Error_reaction::abort: return QObject::tr("Abort");
  case Error_reaction::merge_dir: return QObject::tr("Merge");
  case Error_reaction::delete_existing: return QObject::tr("Delete existing");
  case Error_reaction::rename_existing: return QObject::tr("Rename existing");
  case Error_reaction::continue_writing: return QObject::tr("Continue writing");
  case Error_reaction::rename_new: return QObject::tr("Rename new");
  default:
    qWarning("Error_reaction::to_string failed");
    return QObject::tr("Unknown");
  }

}

QList<Error_reaction::Enum> get_error_reactions(Error_type::Enum error_type,
                                     bool is_dir,
                                     bool is_interactive) {
  if (error_type == Error_type::no_error) {
    qWarning("Error_reaction::get_all: 'no_error' is not allowed");
    return QList<Error_reaction::Enum>();
  }
  QList<Error_reaction::Enum> r;
  r << Error_reaction::skip;
  if (error_type == Error_type::exists) {
    if (is_dir) {
      r << Error_reaction::merge_dir;
    }
    r << Error_reaction::delete_existing
      << Error_reaction::rename_existing
      << Error_reaction::rename_new;
    if (!is_dir) {
      r << Error_reaction::continue_writing;
    }
  }
  r << Error_reaction::abort;
  r.prepend(is_interactive? Error_reaction::retry: Error_reaction::ask);
  return r;
}
