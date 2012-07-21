#include "File_action_task.h"

File_action_task::File_action_task(File_action_type p_action_type, QStringList p_target, QString p_destination):
  action_type(p_action_type),
  target(p_target),
  destination(p_destination),
  recursive_fetch_option(recursive_fetch_auto),
  link_type(link_type_soft_absolute)
{

}

void File_action_task::run() {
  int count = 1e6;
  for(int i = 0; i < count; i++) {
    if (i % 10000 == 0) {
      File_action_state state;
      state.current_action = "testing...";
      state.current_progress = 1.0 * i / count;
      state.total_progress = 0.1;
      emit state_changed(state);
    }
  }

  emit error("not implemented");
  deleteLater();
}
