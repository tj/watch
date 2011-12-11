
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

/*
 * Command version.
 */

#define VERSION "0.0.3"

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
    "    -i, --interval <n>    interval in seconds or ms defaulting to 1\n"
    "    -v, --version         output version number\n"
    "    -h, --help            output this help information\n"
    "\n"
    );
  exit(1);
}

/*
 * Milliseconds string.
 */

int
milliseconds(const char *str) {
  int len = strlen(str);
  return 'm' == str[len-2] && 's' == str[len-1];
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
 * Parse argv.
 */

int
main(int argc, const char **argv){
  if (1 == argc) usage();
  int interval = DEFAULT_INTERVAL;

  int len = 0;
  char *args[ARGS_MAX];

  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];

    // -h, --help
    if (option("-h", "--help", arg)) usage();

    // -q, --quiet
    if (option("-q", "--quiet", arg)) {
      quiet = 1;
      continue;
    }

    // -V, --version
    if (option("-V", "--version", arg)) {
      printf("%s\n", VERSION);
      exit(1);
    }

    // -i, --interval <n>
    if (option("-i", "--interval", arg)) {
      if (argc-1 == i) {
        fprintf(stderr, "\n  --interval requires an argument\n\n");
        exit(1);
      }

      // seconds or milliseconds
      arg = argv[++i];
      interval = milliseconds(arg)
        ? atoi(arg)
        : atoi(arg) * 1000;
      continue;
    }

    // cmd args
    if (len == ARGS_MAX) {
      fprintf(stderr, "number of arguments exceeded %d\n", len);
      exit(1);
    }

    args[len++] = (char *) arg;
  }

  // <cmd>
  if (!len) {
    fprintf(stderr, "\n  <cmd> required\n\n");
    exit(1);
  }

  // null
  args[len] = 0;

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
        execvp(args[0], args);
      // parent
      default:
        if (waitpid(pid, &status, 0) < 0) {
          perror("waitpid()");
          exit(1);
        }

        // exit > 0
        if (WEXITSTATUS(status)) {
          fprintf(stderr, "\033[90mexit: %d\33[0m\n\n", WEXITSTATUS(status));
        } else if (quiet) {
          putchar('.');
          fflush(stdout);
        } else {
          printf("\n");
        }

        mssleep(interval);
        goto loop;
    }    
  }

  return 0;
}
