#ifndef DETAILEDEXCEPTION_H
#define DETAILEDEXCEPTION_H

#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/exception/all.hpp>
#include <boost/stacktrace.hpp>

struct ExceptionDetails {
  boost::stacktrace::stacktrace stackTrace;
  std::string file;
  int line;
};

typedef boost::error_info<struct tag_stacktrace, ExceptionDetails> traced;

/**
 * @brief Throws the exception object with additional details about where the
 * exception orginated from.
 *
 * @note This must be a macro so that I can get the line and file info. Also
 * this makes the first line from the stack trace match the throw site.
 * @note The semicolon is missing on purpose. This way the mcro is used exactly
 * like a function.
 *
 * @todo Make sure __FILE__ returns relative paths so that the user doesn't see
 * paths specific for my PC.
 */
#define throwDetailed(e)                                                       \
  throw boost::enable_error_info(e)                                            \
      << traced({boost::stacktrace::stacktrace(), __FILE__, __LINE__})

/**
 * @brief Returns the details of the exception as a string. Use this to consume
 * the info passed by throwDetailed().
 */
template <class T> std::string catchDetailed(const T &e) {
  std::string str(e.what());

  const ExceptionDetails *st = boost::get_error_info<traced>(e);
  if (nullptr != st) {
    std::stringstream ss;
    ss << st->stackTrace;
    str += " (thrown from " + st->file + ":" + std::to_string(st->line) +
           ")\n\n" + ss.str();
  }

  return str;
}

#endif // DETAILEDEXCEPTION_H
