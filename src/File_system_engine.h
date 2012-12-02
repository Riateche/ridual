#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "File_info.h"

#define FS_ENGINE_BUFFER_SIZE 65535

class File_system_engine : public QObject {
  Q_OBJECT
public:

  class Iterator {
  public:
    virtual bool has_next() = 0;
    File_info get_next();
    File_info get_current() { return current; }
    virtual ~Iterator() {}

  protected:
    Iterator() {}
    virtual File_info get_next_internal() = 0;

  private:
    File_info current;

  };

  class Operation {
  public:
    virtual bool is_finished() = 0;
    virtual double get_progress() = 0;
    virtual void run_iteration() = 0;
    virtual ~Operation() {}

  protected:
    Operation() {}
  };

  class Done_operation: public Operation {
  public:
    bool is_finished() { return true; }
    double get_progress() { return 1.0; }
    void run_iteration() { qWarning("invalid use of run_iteration"); }
  };



  enum error_type {
    unknown_error,
    directory_list_failed,
    cant_open_file_for_read,
    cant_open_file_for_write,
    copy_failed,
    move_failed,
    file_not_seekable,
    file_remove_failed,
    file_read_failed,
    file_write_failed,
    stat_failed,
    mkdir_failed,
    rmdir_failed
  };

  enum error_cause {
    unknown_cause,
    not_found,
    permission_denied,
    already_exists,
    too_many_entries,
    filesystem_full,
    readonly_filesystem,
    busy,
    invalid_path,
    not_directory,
    path_too_long,
    symbolic_links_loop,
    io_error,
    directory_not_empty
  };

  class Exception {
  public:
    Exception() {}
    Exception(error_type type, error_cause cause, QString path1 = QString(), QString path2 = QString());
    inline error_type get_type() { return type; }
    inline error_cause get_cause() { return cause; }
    QString get_message();

  private:
    error_type type;
    error_cause cause;
    QString path1, path2;

  };


  File_system_engine();
  virtual bool is_responsible_for(const QString& uri) = 0;
  virtual Iterator* list(const QString& uri) = 0;
  virtual Operation* copy(const QString& source, const QString& destination) = 0;
  virtual Operation* move(const QString& source, const QString& destination) = 0;
  virtual void remove(const QString& uri) = 0;
  virtual void make_directory(const QString& uri) = 0;
  virtual QString get_real_file_name(const QString& uri) = 0;
  virtual ~File_system_engine() {}


protected:
  static error_cause get_cause_from_errno();


};

#endif // FILE_OPERATIONS_H
