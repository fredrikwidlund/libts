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
  ts_packets packets;
  ts_packet **p;
  ssize_t n;

  if (argc != 2)
    usage();

  ts_packets_construct(&packets);
  n = ts_packets_load(&packets, argv[1]);
  if (n == -1)
    err(1, "ts_packet_load");

  n = 0;
  list_foreach(&packets.list, p)
    {
      (void) fprintf(stdout, "[packet %lu]\n", n);
      (*p)->continuity_counter = 0;
      ts_packet_debug(*p, stdout);
      n ++;
    }

  ts_packets_destruct(&packets);
}
