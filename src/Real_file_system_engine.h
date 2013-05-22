#ifndef REAL_FILE_SYSTEM_ENGINE_H
#define REAL_FILE_SYSTEM_ENGINE_H

#include "File_system_engine.h"
#include <QDirIterator>
#include <fstream>

/*!
 * The Real_file_system_engine class provides access to basic filesystem functions
 * with exception-based error reporting. Now this class supports only Linux, but
 * other OS support may be added here.
 */

class Real_file_system_engine: public File_system_engine {
  Q_OBJECT
public:
  Real_file_system_engine();
  Iterator* list(const QString& uri);
  Operation* copy(const QString& source, const QString& destination, bool append_mode);
  Operation* move(const QString& source, const QString& destination);
  bool is_file(const QString& uri);
  bool is_directory(const QString& uri);

  enum Move_mode {
    move_mode_auto,
    move_mode_copy,
    move_mode_system
  };

  Operation* move(const QString& source, const QString& destination, Move_mode mode);
  void remove(const QString& uri);
  void make_directory(const QString& uri);
  QString get_real_file_name(const QString& uri);
  bool is_responsible_for(const QString& uri);


  class Real_fs_iterator: public Iterator {
  public:
    Real_fs_iterator(QDirIterator* i);
    bool has_next();
    File_info get_next_internal();

  private:
    QDirIterator* iterator;

  };

private:

  class Copy_operation: public Operation {
  public:
    std::ifstream file1;
    std::ofstream file2;
    bool must_delete_source;
    bool finished;
    qint64 file_size;
    QString source_filename;
    QString destination_filename;
    char copy_buffer[FS_ENGINE_BUFFER_SIZE];

    inline bool is_finished() { return finished; }
    double get_progress();
    void run_iteration();
  };

};

#endif // REAL_FILE_SYSTEM_ENGINE_H
