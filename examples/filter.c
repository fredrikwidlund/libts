#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <err.h>

#include <dynamic.h>
#include <ts.h>

void usage()
{
  extern char *__progname;

  (void) fprintf(stderr, "Usage: %s [INPUT FILE] [PID] [OUTPUT FILE]\n", __progname);
  exit(1);
}

int main(int argc, char **argv)
{
  ts_units units;
  ssize_t n;

  if (argc != 4)
    usage();

  ts_units_construct(&units);
  n = ts_units_load(&units, argv[1]);
  if (n == -1)
    err(1, "ts_units_load");

  //ts_units_construct(&pid);
  //ts_units_filter(&units, strtoul(argv[2]), &pid);
  ts_units_destruct(&units);

  /*
  n = ts_units_save(&pid, argv[3]);
  if (n == -1)
    err(1, "ts_units_save");
  ts_units_destruct(&pid);
  */
}
