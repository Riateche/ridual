#ifndef REAL_FILE_SYSTEM_ENGINE_H
#define REAL_FILE_SYSTEM_ENGINE_H

#include "File_system_engine.h"
#include <QDirIterator>
#include <fstream>

class Real_file_system_engine: public File_system_engine {
public:
  Real_file_system_engine();
  Iterator* list(const QString& uri);
  Operation* copy(const QString& source, const QString& destination);
  Operation* move(const QString& source, const QString& destination);
  void remove(const QString& uri);
  void make_directory(const QString& uri);
  QString get_real_file_name(const QString& uri);
  bool is_responsible_for(const QString& uri);

private:
  class Real_fs_iterator: public Iterator {
  public:
    QDirIterator* iterator;
    bool has_next();
    File_info get_next();


  };

  class Copy_operation: public Operation {
  public:
    std::ifstream file1;
    std::ofstream file2;
    bool must_delete_source;
    bool finished;
    qint64 file_size;
    QString source_filename;
    char copy_buffer[FS_ENGINE_BUFFER_SIZE];

    inline bool is_finished() { return finished; }
    double get_progress();
    void run_iteration();
  };

};

#endif // REAL_FILE_SYSTEM_ENGINE_H
