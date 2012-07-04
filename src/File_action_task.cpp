#include "File_action_task.h"

File_action_task::File_action_task(File_action_type p_action, QStringList p_target, QString p_destination, Recursive_fetch_option p_recursive_fetch_option):
  action(p_action),
  target(p_target),
  destination(p_destination),
  recursive_fetch_option(p_recursive_fetch_option)
{

}

void File_action_task::run() {
  emit error("not implemented");
  deleteLater();
}
