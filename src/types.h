#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QVariant>
#include "File_info.h"
#include "File_system_engine.h"

namespace Action_type {
  enum Enum {
    copy,
    move,
    link,
    remove,
    trash,
    make_directory
  };
}

enum Recursive_fetch_option {
  recursive_fetch_on = 1,
  recursive_fetch_off = 2,
  recursive_fetch_auto = 3
};

enum Link_type {
  link_type_soft_absolute,
  link_type_soft_relative,
  link_type_hard
};

class Action_data {
public:
  Action_data() : type(Action_type::copy), recursive_fetch_option(recursive_fetch_auto),
    link_type(link_type_soft_absolute), destination_includes_filename(false) {}
  Action_type::Enum type;
  Recursive_fetch_option recursive_fetch_option;
  Link_type link_type;
  File_info_list targets;
  QString destination;
  bool destination_includes_filename;

};

class Action_state {
public:
  Action_state(): queue_id(0) {}
  QString current_action;
  QString current_progress_text;
  QString total_progress_text;
  double current_progress, total_progress; // from 0 to 1, or -1 (unknown) or -2 (disabled)
  //int errors_count;
  int queue_id;
  static const int UNKNOWN = -1;
  static const int DISABLED = -2;
};
Q_DECLARE_METATYPE(Action_state)

namespace Error_type {
  enum Enum {
    destination_inside_source,
    file_system_error
  };
}
Q_DECLARE_METATYPE(Error_type::Enum)

class Action;

class Question_data {
public:
  Question_data() {}
  Question_data(Action* a, Error_type::Enum t) :
    action(a),
    error_type(t) {}
  Question_data(Action* a, File_system_engine::Exception e) :
    action(a),
    error_type(Error_type::file_system_error),
    fs_exception(e) {}

  QString get_message() {
    switch(error_type) {
    case Error_type::destination_inside_source:
      return QObject::tr("Cannot copy a directory inside itself.");
    case Error_type::file_system_error:
      return fs_exception.get_message();
    }
    return QString();
  }

  Action* action;
  Error_type::Enum error_type;
  File_system_engine::Exception fs_exception;
};
Q_DECLARE_METATYPE(Question_data)

namespace Icon {
  enum Enum {
    warning,
    info,
    error,
    success
  };
}

Q_DECLARE_METATYPE(Icon::Enum)




#endif // TYPES_H
