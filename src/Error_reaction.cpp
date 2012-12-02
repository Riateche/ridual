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

QList<Error_reaction::Enum> get_error_reactions(Question_data data, bool is_interactive) {
  QList<Error_reaction::Enum> r;
  if (data.error_type == Error_type::destination_inside_source) {
    r << Error_reaction::abort;
  } else if (data.error_type == Error_type::file_system_error) {
    File_system_engine::error_type fs_type = data.fs_exception.get_type();
    r << Error_reaction::skip;
    //todo: add possible reactions
/*    if (fs_type == File_system_engine::) {
      if (is_dir) {
        r << Error_reaction::merge_dir;
      }
      r << Error_reaction::delete_existing
        << Error_reaction::rename_existing
        << Error_reaction::rename_new;
      if (!is_dir) {
        r << Error_reaction::continue_writing;
      }
    } */
    r << Error_reaction::abort;
    r.prepend(is_interactive? Error_reaction::retry: Error_reaction::ask);

  }

  return r;
}
