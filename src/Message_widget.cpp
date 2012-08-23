#include "Message_widget.h"
#include <QHBoxLayout>
#include <QLabel>

Message_widget::Message_widget(QString message, Icon::Enum icon)
{
  setFrameShape(QFrame::StyledPanel);
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
  icon_label->setPixmap(QPixmap(QString(":/images/%1.png").arg(icon_name)));
  layout->addWidget(icon_label);
  layout->addWidget(new QLabel(message));
  layout->addStretch();
}
