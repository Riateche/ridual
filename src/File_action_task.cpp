#include "File_action_task.h"

File_action_task::File_action_task(const File_action_type& p_action_type, File_info_list p_target, QString p_destination):
  action_type(p_action_type),
  target(p_target),
  destination(p_destination),
  recursive_fetch_option(recursive_fetch_auto),
  link_type(link_type_soft_absolute),
  queue(0)
{
  qRegisterMetaType<File_action_state>("File_action_state");

}

void File_action_task::run(File_action_queue *p_queue) {
  queue = p_queue;
  int count = 1e8;
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
