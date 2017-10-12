#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>

#include <dynamic.h>
#include <ts.h>

void usage()
{
  extern char *__progname;

  (void) fprintf(stderr, "Usage: %s [INPUT FILE] [PID]\n", __progname);
  exit(1);
}

int main(int argc, char **argv)
{
  ts_stream stream;
  ts_stream_pes *s;
  ts_pes **p;
  ssize_t n;
  int pid;

  if (argc != 3)
    usage();

  ts_stream_construct(&stream);
  n = ts_stream_load(&stream, argv[1]);
  if (n == -1)
    err(1, "ts_stream_load");
  pid = strtoul(argv[2], NULL, 0);

  s = ts_stream_lookup(&stream, pid);
  if (!s)
    errx(1, "no such pid");

  list_foreach(&s->packets, p)
    {
      n = fwrite((*p)->data, (*p)->size, 1, stdout);
      if (n != 1)
        err(1, "fwrite");
    }

  ts_stream_destruct(&stream);
}
