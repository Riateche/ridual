#include "Task.h"

Task::Task(Main_window *mw, const Action_data &p_data):
  main_window(mw),
  data(p_data)
{
  total_size = 0;
  current_size = 0;
  total_count = 0;
  current_count = 0;
  current_root = 0;
  current_item = 0;

  qRegisterMetaType<Action_state>("Action_state");
  connect(this, SIGNAL(error(QString)), main_window, SLOT(fatal_error(QString)));
  connect(this, SIGNAL(error(QString)), this, SIGNAL(finished()));
  connect(&iteration_timer, SIGNAL(timeout()), this, SLOT(iteration()));
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

Task::~Task()
{
}

QString Task::get_real_dir(QString uri) {
  if (uri.startsWith("/")) return uri;
  foreach(gio::Mount mount, mounts) {
    if (!mount.uri.isEmpty() && uri.startsWith(mount.uri)) {
      //convert uri e.g. "ftp://user@host/path"
      //to real path e.g. "/home/user/.gvfs/FTP as user on host/path"
      QString real_dir = mount.path + "/" + uri.mid(mount.uri.length());
      return real_dir;
    }
  }
  return uri;
}

void Task::run() {
  mounts = main_window->get_gio_mounts(); //thread-safe
  if (!data.destination.isEmpty()) {
    data.destination = get_real_dir(data.destination);
  }

  if (data.type != action_copy) {
    emit error("not implemented");
    iteration_timer.stop();
    return;
  }

  foreach(File_info target, data.targets) {
//    leafs << new File_leaf(target.full_name, target.is_folder(), target.parent_folder);
  }

  state.current_action = tr("Starting");
  emit state_changed(tmp_state);
  //...
}
