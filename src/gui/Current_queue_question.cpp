#include "Current_queue_question.h"
#include "Main_window.h"
#include "Action_queue.h"
#include "Core.h"
#include "Actions_manager.h"

Current_queue_question::Current_queue_question(Main_window *mw) :
  Question_widget(mw)
{
  set_message(tr("Choose a queue for the next task:"));
  QList<Button_settings> list;
  list << Button_settings(0, tr("New queue (default)"), 0);
  QHash<int, Action_queue*> queues = main_window->get_core()->get_actions_manager()->get_queues_hash();
  foreach(int id, queues.keys()) {
    list << Button_settings(id, tr("Queue %1").arg(id), id);
  }
  set_buttons(list);
}

void Current_queue_question::answered(QVariant data) {
  int id = data.toInt();
  if (id != 0) {
    QHash<int, Action_queue*> queues = main_window->get_core()->get_actions_manager()->get_queues_hash();
    if (queues.contains(id)) {
      main_window->set_current_queue(queues[id]);
      return;
    }
  }
  main_window->set_current_queue(0);
}
