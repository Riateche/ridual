#include "Message_widget.h"
#include <QHBoxLayout>
#include <QLabel>

Message_widget::Message_widget(QString message, Icon::Enum icon) {
  setFrameShape(QFrame::StyledPanel);
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(5, 3, 5, 3);
  QLabel* icon_label = new QLabel();
  QString icon_name;
  switch(icon) {
    case Icon::warning: icon_name = "warning";  break;
    case Icon::error:   icon_name = "error";    break;
    case Icon::success: icon_name = "success";  break;
    case Icon::info:    icon_name = "info";     break;
  }
  icon_label->setPixmap(QPixmap(QString(":/resources/images/%1.png").arg(icon_name)));
  layout->addWidget(icon_label);
  QLabel* message_label = new QLabel(message);
  message_label->setWordWrap(true);
  layout->addWidget(message_label, 10);
  layout->addStretch();
}
