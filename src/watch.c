
//
// watch.c
//
// Copyright (c) 2011 TJ Holowaychuk <tj@vision-media.ca>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "ms/ms.h"

/*
 * Command version.
 */

#define VERSION "0.3.1"

/*
 * Default interval in milliseconds.
 */

#define DEFAULT_INTERVAL 1000

/*
 * Max command args.
 */

#define ARGS_MAX 128

/*
 * Quiet mode.
 */

static int quiet = 0;

/*
 * Halt on failure.
 */

static int halt = 0;

/*
 * Output command usage.
 */

void
usage() {
  printf(
    "\n"
    "  Usage: watch [options] <cmd>\n"
    "\n"
    "  Options:\n"
    "\n"
    "    -q, --quiet           only output stderr\n"
    "    -x, --halt            halt on failure\n"
    "    -i, --interval <n>    interval in seconds or ms defaulting to 1\n"
    "    -v, --version         output version number\n"
    "    -h, --help            output this help information\n"
    "\n"
    );
  exit(1);
}

/*
 * Sleep in `ms`.
 */

void
mssleep(int ms) {
  struct timespec req = {0};
  time_t sec = (int)(ms / 1000);
  ms = ms -(sec * 1000);
  req.tv_sec = sec;
  req.tv_nsec = ms * 1000000L;
  while(-1 == nanosleep(&req, &req)) ;
}

/*
 * Redirect stdout to `path`.
 */

void
redirect_stdout(const char *path) {
  int fd = open(path, O_WRONLY);
  if (dup2(fd, 1) < 0) {
    perror("dup2()");
    exit(1);
  }
}

/*
 * Check if `arg` is the given short-opt or long-opt.
 */

int
option(char *small, char *large, const char *arg) {
  if (!strcmp(small, arg) || !strcmp(large, arg)) return 1;
  return 0;
}

/*
 * Return the total string-length consumed by `strs`.
 */

int
length(char **strs) {
  int n = 0;
  char *str;
  while ((str = *strs++)) n += strlen(str);
  return n + 1;
}

/*
 * Join the given `strs` with `val`.
 */

char *
join(char **strs, int len, char *val) {
  --len;
  char *buf = calloc(1, length(strs) + len * strlen(val) + 1);
  char *str;
  while ((str = *strs++)) {
    strcat(buf, str);
    if (*strs) strcat(buf, val);
  }
  return buf;
}

/*
 * Parse argv.
 */

int
main(int argc, const char **argv){
  if (1 == argc) usage();
  int interval = DEFAULT_INTERVAL;

  int len = 0;
  int interpret = 1;
  char *args[ARGS_MAX] = {0};

  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    if (!interpret) goto arg;

    // -h, --help
    if (option("-h", "--help", arg)) usage();

    // -q, --quiet
    if (option("-q", "--quiet", arg)) {
      quiet = 1;
      continue;
    }

    // -x, --halt
    if (option("-x", "--halt", arg)) {
      halt = 1;
      continue;
    }

    // -v, --version
    if (option("-v", "--version", arg)) {
      printf("%s\n", VERSION);
      exit(1);
    }

    // -i, --interval <n>
    if (option("-i", "--interval", arg)) {
      if (argc-1 == i) {
        fprintf(stderr, "\n  --interval requires an argument\n\n");
	exit(1);
      }

      arg = argv[++i];
      char last = arg[strlen(arg) - 1];
      // seconds or milliseconds
      interval = last >= 'a' && last <= 'z'
	? string_to_milliseconds(arg)
	: atoi(arg) * 1000;
      continue;
    }

    // cmd args
    if (len == ARGS_MAX) {
      fprintf(stderr, "number of arguments exceeded %d\n", len);
      exit(1);
    }

  arg:
    args[len++] = (char *) arg;
    interpret = 0;
  }

  // <cmd>
  if (!len) {
    fprintf(stderr, "\n  <cmd> required\n\n");
    exit(1);
  }

  // cmd
  char *val = join(args, len, " ");
  char *cmd[4] = { "sh", "-c", val, 0 };

  // exec loop
  loop: {
    pid_t pid;
    int status;
    switch (pid = fork()) {
      // error
      case -1:
        perror("fork()");
        exit(1);
      // child
      case 0:
        if (quiet) redirect_stdout("/dev/null");
        execvp(cmd[0], cmd);
      // parent
      default:
        if (waitpid(pid, &status, 0) < 0) {
          perror("waitpid()");
          exit(1);
        }

        // exit > 0
        if (WEXITSTATUS(status)) {
          fprintf(stderr, "\033[90mexit: %d\33[0m\n\n", WEXITSTATUS(status));
	  if (halt) exit(WEXITSTATUS(status));
	}

	mssleep(interval);
	goto loop;
    }
  }

  return 0;
}
