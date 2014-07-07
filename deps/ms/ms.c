
//
// ms.c
//
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "asprintf/asprintf.h"
#include "ms.h"

// microseconds

#define US_MSEC (long long) 1000
#define US_SEC 1000 * US_MSEC
#define US_MIN 60 * US_SEC
#define US_HOUR 60 * US_MIN
#define US_DAY 24 * US_HOUR
#define US_WEEK 7 * US_DAY
#define US_YEAR 52 * US_WEEK

// milliseconds

#define MS_SEC (long long)1000
#define MS_MIN 60 * MS_SEC
#define MS_HOUR 60 * MS_MIN
#define MS_DAY 24 * MS_HOUR
#define MS_WEEK 7 * MS_DAY
#define MS_YEAR 52 * MS_WEEK

/*
 * Convert the given `str` representation to microseconds,
 * for example "10ms", "5s", "2m", "1h" etc.
 */

long long
string_to_microseconds(const char *str) {
  size_t len = strlen(str);
  long long val = strtoll(str, NULL, 10);
  if (!val) return -1;
  switch (str[len - 1]) {
    case 's': return  'm' == str[len - 2] ? val * 1000 : val * US_SEC;
    case 'm': return val * US_MIN;
    case 'h': return val * US_HOUR;
    case 'd': return val * US_DAY;
    case 'w': return val * US_WEEK;
    case 'y': return val * US_YEAR;
    default:  return val;
  }
}

/*
 * Convert the given `str` representation to milliseconds,
 * for example "10ms", "5s", "2m", "1h" etc.
 */

long long
string_to_milliseconds(const char *str) {
  size_t len = strlen(str);
  long long val = strtoll(str, NULL, 10);
  if (!val) return -1;
  switch (str[len - 1]) {
    case 's': return  'm' == str[len - 2] ? val : val * 1000;
    case 'm': return val * MS_MIN;
    case 'h': return val * MS_HOUR;
    case 'd': return val * MS_DAY;
    case 'w': return val * MS_WEEK;
    case 'y': return val * MS_YEAR;
    default:  return val;
  }
}

/*
 * Convert the given `str` representation to seconds.
 */

long long
string_to_seconds(const char *str) {
  long long ret = string_to_milliseconds(str);
  if (-1 == ret) return ret;
  return ret / 1000;
}

/*
 * Convert the given `ms` to a string. This
 * value must be `free()`d by the developer.
 */

char *
milliseconds_to_string(long long ms) {
  char *str = NULL;
  long long div = 1;
  char *fmt;

  if (ms < MS_SEC) fmt = "%lldms";
  else if (ms < MS_MIN) { fmt = "%llds"; div = MS_SEC; }
  else if (ms < MS_HOUR) { fmt = "%lldm"; div = MS_MIN; }
  else if (ms < MS_DAY) { fmt = "%lldh"; div = MS_HOUR; }
  else if (ms < MS_WEEK) { fmt = "%lldd"; div = MS_DAY; }
  else if (ms < MS_YEAR) { fmt = "%lldw"; div = MS_WEEK; }
  else { fmt = "%lldy"; div = MS_YEAR; }
  if (-1 == asprintf(&str, fmt, ms / div)) {
    return NULL;
  }

  return str;
}

/*
 * Convert the given `ms` to a long string. This
 * value must be `free()`d by the developer.
 */

char *
milliseconds_to_long_string(long long ms) {
  long long div;
  char *name;

  char *str = NULL;

  if (ms < MS_SEC) {
    if (-1 == asprintf(&str, "less than one second")) return NULL;
    return str;
  }

  if (ms < MS_MIN) { name = "second"; div = MS_SEC; }
  else if (ms < MS_HOUR) { name = "minute"; div = MS_MIN; }
  else if (ms < MS_DAY) { name = "hour"; div = MS_HOUR; }
  else if (ms < MS_WEEK) { name = "day"; div = MS_DAY; }
  else if (ms < MS_YEAR) { name = "week"; div = MS_WEEK; }
  else { name = "year"; div = MS_YEAR; }

  long long val = ms / div;
  char *fmt = 1 == val
    ? "%lld %s"
    : "%lld %ss";

  if (-1 == asprintf(&str, fmt, val, name)) {
    return NULL;
  }

  return str;
}
