#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QVariant>
#include "File_info.h"

enum Action_type {
  action_copy,
  action_move,
  action_link,
  action_delete,
  action_create_folder
};

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
  Action_type type;
  Recursive_fetch_option recursive_fetch_option;
  Link_type link_type;
  File_info_list targets;
  QString destination;
};

class Action_state {
public:
  Action_state() : errors_count(0), queue_id(0) {}
  QString current_action, current_progress, total_progress;
  int errors_count;
  int queue_id;
};
Q_DECLARE_METATYPE(Action_state)

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

class Action;
class Question_data {
public:
  Question_data() {}
  Question_data(QString m, Error_type t, bool d) :
    message(m)
  , error_type(t)
  , is_dir(d) {}

  QString message;
  Error_type error_type;
  bool is_dir;
  Action* action;
};
Q_DECLARE_METATYPE(Question_data)



#endif // TYPES_H
