#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>

#include <dynamic.h>
#include <ts.h>

void usage()
{
  extern char *__progname;

  (void) fprintf(stderr, "Usage: %s [INPUT FILE]\n", __progname);
  exit(1);
}

int main(int argc, char **argv)
{
  ts_units units;
  ts_unit **u;
  ssize_t n;

  if (argc != 2)
    usage();

  ts_units_construct(&units);
  n = ts_units_load(&units, argv[1]);
  if (n == -1)
    err(1, "ts_units_load");

  n = 0;
  list_foreach(ts_units_list(&units), u)
    {
      (void) fprintf(stdout, "[unit %lu]\n", n);
      ts_unit_debug(*u, stdout);
      n ++;
    }

  ts_units_destruct(&units);
}
