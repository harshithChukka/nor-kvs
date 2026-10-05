#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

typedef enum
{
  LOG_ERROR = 0,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG
} LogLevel;

typedef struct
{
  LogLevel log_level;
  const char *label;
} logger_t;

static const char *level_to_string(LogLevel level)
{
  switch (level)
  {
  case LOG_ERROR:
    return "ERROR";

  case LOG_WARN:
    return "WARN";

  case LOG_INFO:
    return "INFO";

  case LOG_DEBUG:
    return "DEBUG";

  default:
    return "UNKNOWN";
  }
}

void logger_init(logger_t *logger, LogLevel level, const char *label)
{
  logger->label = label;
  logger->log_level = level;
}

void logger_log(logger_t *logger, LogLevel level, const char *fmt, ...)
{
  if (level > logger->log_level)
    return;

  printf("[%s_%s] ", logger->label, level_to_string(level));

  va_list args;

  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  printf("\n");
}

#endif
