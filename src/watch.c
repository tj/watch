#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdbool.h>

#define VERSION "0.0.1"
#define PROGNAME "watch"
#define MAX_ARGS 128

static char * progname = NULL;

static struct option option_tab[] = {
  { "help", no_argument, 0, 'h' },
  { "version", no_argument, 0, 'v' },
  { "interval", required_argument, 0, 'i' },
  { "count", required_argument, 0, 'c' },
  { "output", required_argument, 0, 'o' },
  { "halt", no_argument, 0, 'x' },
  { "quiet", no_argument, 0, 'q' },
  { "timestamp", no_argument, 0, 't' },
  { "clear-screen", no_argument, 0, 'l' },
  { (const char *)0, 0, 0, 0 }
};

void usage ()
{
  printf ("Usage: %s [-vhqHlt] [-i <n[ms]>] [-c <count>] [-o <output_path>] <command>\n", progname);
  exit (1);
}

void version ()
{
  printf ("%s\n", VERSION);
  exit (1);
}

bool is_interval_ms (const char * interval)
{
  int last = strlen(interval) - 1;
  return ((interval[last] == 's') && (interval[last - 1] == 'm'));  
}

void mssleep (int ms)
{
  struct timespec req = {0};
  time_t seconds = (int) (ms / 1000);
  ms = ms - (seconds * 1000);
  req.tv_sec = seconds;
  req.tv_nsec = ms * 1000000L;
  while (-1 == nanosleep (&req, NULL));
}

void redirect_output (const char *path)
{
  int fd = open(path, O_CREAT|O_WRONLY|O_APPEND);
  if (fd == -1)
  {
    perror ("Unable to open output file");
    return;
  }

  fflush(stdout);

  if (dup2(fd, 1) < 0)
  {
    perror ("Unable to redirect output");
    return;
  }
}

int main (int argc, char * const argv[])
{
  int interval = 1000;
  int arg_number = 0;
  char * command[MAX_ARGS] = {0};
  int option = 0;
  int count = 0;
  progname = argv[0];
  bool quiet = false;
  bool halt = false;
  bool clear_screen = false;
  bool timestamped = false;
  char * output_path = NULL;

  while (( option = getopt_long (argc, argv, "+hvi:c:qxlto:", option_tab, 0) ) != -1 )
  {
    switch (option)
    {
    case 'h':
      usage ();
      break;
    case 'v':
      version ();
      break;
    case 'c':
	{
	  char * endptr = NULL;
	  strtol(optarg, &endptr, 10);
	  if (endptr != NULL && endptr < (optarg + strlen(optarg)))
	  {
		fprintf (stderr, "[-c <number>]\n");
        usage ();
	  }
      count = atoi(optarg);
	}
      break;
    case 'x':
      halt = true;
      break;
    case 'q':
      if (output_path != NULL)
      {
        fprintf (stderr, "[-q|-f <output_file>]\n");
        usage ();
      }
      quiet = true;
      break;
    case 't':
      timestamped = true;
      break;
    case 'o':
      if (quiet == true)
      {
        fprintf (stderr, "[-q|-f <output_file>]\n");
        usage ();
      }
      output_path = optarg;
      break;
    case 'i':
      interval = ((is_interval_ms (optarg)) ? atoi (optarg) : atoi (optarg) * 1000);
      break;
	case 'l':
      clear_screen = true;
      break;
    default:
      fprintf (stderr, "Invalid argument\n");
      usage ();
      break;
    }
  }

  while ((optind < argc) && (arg_number < MAX_ARGS))
  {
    command[arg_number++] = argv[optind++];
  }

  if (arg_number == 0)
  {
    fprintf (stderr, "No command provided\n");
    usage ();
  }

  if (arg_number == MAX_ARGS)
  {
    fprintf (stderr, "Long command provided\n");
    usage ();
  }

  loop:
  {
    pid_t pid = 0;
    int status = 0;
    static unsigned long long int repet = 0;
    switch (pid = fork ())
    {
    case -1:
      perror ("fork()");
      return EXIT_FAILURE;
    case 0:
    {
      char * buffer = (char *) malloc (512 * sizeof (char));

      if (quiet == true)
      {
        redirect_output ("/dev/null");
      }
      else if (output_path != NULL)
      {
        redirect_output (output_path);
      }
	  
	  if (clear_screen == true && output_path == NULL)
	  {
#ifdef _WIN32
		system("cls");
#else
		system("clear");
#endif
	  }

      if (timestamped == true)
      {
        char date[24] = {0};
        time_t tm;
        time (&tm);
        strftime (date, 24, "%F %H:%M:%S", localtime (&tm));
        sprintf (buffer, "(%llu) %s]===>\n", repet, date);
      }
      else
      {
        sprintf (buffer, "===>\n");
      }

      write (1, buffer, strlen (buffer));

      free (buffer);

      if(execvp (command[0], command) == -1) {
		perror ("execvp()");
		return EXIT_FAILURE;
	  }
    }
      break;
    default:
      if (waitpid (pid, &status, 0) < 0)
      {
        perror ("waitpid()");
        return EXIT_FAILURE;
      }

      if ((halt == true) && (WEXITSTATUS (status) != 0))
      {
        exit (WEXITSTATUS (status));
      }

      if (timestamped == true)
      {
        repet++;
      }
      break;
    }

    if ((count != 0) && (--count == 0))
    {
      goto exit;
    }

    mssleep (interval);
  }
  goto loop;

  exit:
  return EXIT_SUCCESS;
}

