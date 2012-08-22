#include "Current_queue_question.h"
#include "Main_window.h"
#include "Action_queue.h"

Current_queue_question::Current_queue_question(Main_window *mw) :
  Question_widget(mw)
{
  set_message(tr("Choose a queue for the next task:"));
  QList<Button_settings> list;
  list << Button_settings(0, tr("New queue (default)"), 0);
  foreach(Action_queue* queue, main_window->get_queues()) {
    int id = queue->get_id();
    list << Button_settings(id, tr("Queue %1").arg(id), id);
  }
  set_buttons(list);
}

void Current_queue_question::answered(QVariant data) {
  int id = data.toInt();
  if (id != 0) {
    foreach(Action_queue* queue, main_window->get_queues()) {
      if (id == queue->get_id()) {
        main_window->set_current_queue(queue);
        return;
      }
    }
  }
  main_window->set_current_queue(0);
}
