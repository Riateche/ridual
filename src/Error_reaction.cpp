#include "Error_reaction.h"
#include <QDebug>

QString error_reaction_to_string(Error_reaction::Enum reaction) {
  switch (reaction) {
  case Error_reaction::ask: return QObject::tr("Ask");
  case Error_reaction::retry: return QObject::tr("Retry");
  case Error_reaction::skip: return QObject::tr("Skip");
  case Error_reaction::abort: return QObject::tr("Abort");
  case Error_reaction::merge_dir: return QObject::tr("Merge");
  case Error_reaction::overwrite: return QObject::tr("Overwrite");
  case Error_reaction::rename_existing: return QObject::tr("Rename existing");
  case Error_reaction::continue_writing: return QObject::tr("Continue writing");
  case Error_reaction::rename_new: return QObject::tr("Rename new");
  case Error_reaction::delete_competely: return QObject::tr("Delete completely");
  default:
    qWarning("Error_reaction::to_string failed");
    return QObject::tr("Unknown");
  }
}

QList<Error_reaction::Enum> get_error_reactions(Question_data data, bool is_interactive) {
  File_system_engine::error_type type = data.fs_exception.get_type();
  QList<Error_reaction::Enum> r;
  if (data.error_type == Error_type::file_system_error &&
      type == File_system_engine::move_to_trash_failed) {
    r << Error_reaction::delete_competely;
    r << Error_reaction::abort;
    if (!is_interactive) { r << Error_reaction::ask; }
    return r;
  }
  r << Error_reaction::abort;
  r << Error_reaction::skip;

  //if (data.error_type == Error_type::destination_inside_source) {
      // nothing?
  //} else
  if (data.error_type == Error_type::file_system_error) {
    r.prepend(is_interactive? Error_reaction::retry: Error_reaction::ask);
    File_system_engine::error_cause cause = data.fs_exception.get_cause();
    if (cause == File_system_engine::file_already_exists) {
      if (type == File_system_engine::copy_failed) {
        r << Error_reaction::continue_writing;
      }
      r << Error_reaction::overwrite;
      r << Error_reaction::rename_existing;
      r << Error_reaction::rename_new;
    }
    if (cause == File_system_engine::directory_already_exists) {
      r << Error_reaction::merge_dir;
      r << Error_reaction::overwrite;
      r << Error_reaction::rename_existing;
      r << Error_reaction::rename_new;
    }
  }
  return r;
}
