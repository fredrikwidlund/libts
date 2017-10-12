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
  ts_stream stream;
  ssize_t n;

  if (argc != 2)
    usage();


  ts_stream_construct(&stream);
  n = ts_stream_load(&stream, argv[1]);
  if (n == -1)
    err(1, "ts_stream_load");

  ts_stream_debug(&stream, stdout);

  ts_stream_destruct(&stream);
}
