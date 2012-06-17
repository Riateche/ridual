#include "File_list_model.h"
#include <QIcon>

File_list_model::File_list_model()
{
}

void File_list_model::set_data(QFileInfoList p_list) {
  emit layoutAboutToBeChanged();
  list = p_list;
  emit layoutChanged();
}

int File_list_model::rowCount(const QModelIndex &parent) const {
  return list.count();
  Q_UNUSED(parent);
}

QVariant File_list_model::data(const QModelIndex &index, int role) const {
  if (role == Qt::DisplayRole) {
    return list.at(index.row()).fileName();
  }
  if (role == Qt::DecorationRole) {
    QImage image(5, 5, QImage::Format_ARGB32);
    image.fill(Qt::green);
    image.setPixel(2, 2, Qt::white);
    return QPixmap::fromImage(image);
  }
  return QVariant();
}

QFileInfo File_list_model::info(const QModelIndex &index) {
  return list[index.row()];
}
