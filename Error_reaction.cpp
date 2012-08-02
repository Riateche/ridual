#include "Error_reaction.h"
#include <QDebug>

Error_reaction::Error_reaction(Reaction r): reaction(r)
{
}

QString Error_reaction::get_text() {
  switch (reaction) {
    case ask: return QObject::tr("Ask");
    case retry: return QObject::tr("Retry");
    case skip: return QObject::tr("Skip");
    case abort: return QObject::tr("Abort");
    case merge_dir: return QObject::tr("Merge");
    case delete_existing: return QObject::tr("Delete existing");
    case rename_existing: return QObject::tr("Rename existing");
    case continue_writing: return QObject::tr("Continue writing");
    case rename_new: return QObject::tr("Rename new");
  }
  qWarning("Error_reaction::get_text failed");
  return QObject::tr("Unknown");
}

QList<Error_reaction> Error_reaction::get_all(Error_reaction::Error_type error_type,
                                              bool is_interactive) {
  QList<Error_reaction> r;
  r << skip;
  if (error_type == error_type_dir_exists) {
    r << merge_dir;
  }
  if (error_type != error_type_rw) {
    r << delete_existing << rename_existing << rename_new;
  }
  if (error_type == error_type_file_exists) {
    r << continue_writing;
  }
  r << abort;
  r.prepend(is_interactive? retry: ask);
  return r;
}
