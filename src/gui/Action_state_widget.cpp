#include "Action_state_widget.h"
#include "ui_Action_state_widget.h"
#include "Action.h"
#include <QTextDocument>
#include <QDebug>

Action_state_widget::Action_state_widget(Action* action) :
  ui(new Ui::Action_state_widget)
, paused(false)
{
  ui->setupUi(this);
  icon_pause = QIcon(":/resources/images/pause.png");
  icon_resume = QIcon(":/resources/images/play.png");
  connect(action, SIGNAL(started()), this, SIGNAL(show_requested()));
  connect(action, SIGNAL(state_changed(Action_state)),
          this, SLOT(state_changed(Action_state)));
  connect(action, SIGNAL(destroyed()), this, SLOT(deleteLater()));
  connect(this, SIGNAL(set_paused(bool)), action, SLOT(set_paused(bool)));
  connect(this, SIGNAL(state_received()), action, SLOT(state_delivered()));
  connect(ui->cancel, SIGNAL(clicked()), action, SLOT(abort()));
  connect(this, SIGNAL(signal_abort()), action, SLOT(abort()));
}

Action_state_widget::~Action_state_widget() {
  delete ui;
}

void Action_state_widget::abort() {
  emit signal_abort();
}

void Action_state_widget::state_changed(Action_state state) {
  QString html = state.current_action;
#if QT_VERSION >= 0x050000
  html = html.toHtmlEscaped();
#else
  html = Qt::escape(html);
#endif
  html = html.replace("/", "/&#8203;");
  ui->message->setText(html);

  QStringList texts; texts << state.current_progress_text << state.total_progress_text;
  QList<double> values; values << state.current_progress << state.total_progress;
  QList<QLabel*> labels; labels << ui->current_text << ui->total_text;
  QList<QProgressBar*> bars; bars << ui->current_progress << ui->total_progress;
  QStringList captions; captions << tr("Current file: ") << tr("Total: ");
  for(int i = 0; i <= 1; i++) {
    double value = values[i];
    QString text = texts[i];
    QLabel* l = labels[i];
    QProgressBar* bar = bars[i];
    QString caption = captions[i];
    bool unknown = value == Action_state::UNKNOWN;
    bool disabled = value == Action_state::DISABLED;

    if (value >= 0 && value <= 1) {
      text = "";
    }
    bar->setEnabled(!disabled);
    bar->setRange(0, unknown? 0: 100);
    bar->setValue(unknown || disabled? 0: qRound(value * 100));
    if (disabled) bar->reset();
    l->setText(disabled? "": caption + text);
  }
  emit state_received();
  //qDebug() << "emit state_received()";
}

void Action_state_widget::on_pause_clicked() {
  paused = !paused;
  qDebug() << paused;
  ui->pause->setIcon(paused ? icon_resume : icon_pause);
  emit set_paused(paused);
}

