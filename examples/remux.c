#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>

#include <dynamic.h>
#include <ts.h>

void usage()
{
  extern char *__progname;

  (void) fprintf(stderr, "Usage: %s [INPUT FILE] [OUTPUT FILE]\n", __progname);
  exit(1);
}

int main(int argc, char **argv)
{
  ts_packets packets;
  ssize_t n;

  if (argc != 3)
    usage();

  ts_packets_construct(&packets);
  n = ts_packets_load(&packets, argv[1]);
  if (n == -1)
    err(1, "ts_packet_load");

  n = ts_packets_save(&packets, argv[2]);
  if (n == -1)
    err(1, "ts_packet_save");
  ts_packets_destruct(&packets);
}
