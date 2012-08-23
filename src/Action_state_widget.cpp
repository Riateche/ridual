#include "Action_state_widget.h"
#include "ui_Action_state_widget.h"
#include "Action.h"

Action_state_widget::Action_state_widget(Action* action) :
  ui(new Ui::Action_state_widget)
{
  ui->setupUi(this);
  connect(action, SIGNAL(state_changed(Action_state)),
          this, SLOT(state_changed(Action_state)));
  connect(action, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

Action_state_widget::~Action_state_widget() {
  delete ui;
}

void Action_state_widget::state_changed(Action_state state) {
  ui->message->setText(state.current_action);
  QVariantList values; values << state.current_progress << state.total_progress;
  QList<QLabel*> labels; labels << ui->current_text << ui->total_text;
  QList<QProgressBar*> bars; bars << ui->current_progress << ui->total_progress;
  QStringList captions; captions << tr("Current file: ") << tr("Total: ");
  for(int i = 0; i <= 1; i++) {
    QVariant v = values[i];
    if (v.type() == QVariant::String) {
      labels[i]->setText(v.toString());
    } else if (v.type() == QVariant::Bool && v.toBool() == false) {
      labels[i]->clear();
    } else {
      labels[i]->setText(captions[i]);
    }
    if (v.type() == QVariant::Double) {
      bars[i]->setRange(0, 100);
      bars[i]->setValue(qRound(v.toDouble() * 100));
      bars[i]->setEnabled(true);
    } else {
      if (i == 0) {
        bars[i]->setRange(0, 100);
        bars[i]->setValue(0);
        bars[i]->setEnabled(false);
      } else {
        bars[i]->setRange(0, 0);
        bars[i]->setValue(0);
        bars[i]->setEnabled(true);
      }
    }

  }

}
