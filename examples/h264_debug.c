#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#include <dynamic.h>
#include <ts.h>

static const char *desc_type[] = {"Unspecified", "Coded slice of a non-IDR picture", "Coded slice data partition A",
                                  "Coded slice data partition B", "Coded slice data partition C", "Coded slice of an IDR picture",
                                  "Supplemental enhancement information (SEI)", "Sequence parameter set", "Picture parameter set",
                                  "Access unit delimiter", "End of sequence", "End of stream", "Filler data",
                                  "Sequence parameter set extension", "Prefix NAL unit", "Subset sequence parameter set",
                                  "Reserved", "Reserved", "Reserved", "Coded slice of an auxiliary coded picture without partitionin",
                                  "Coded slice extension", "Coded slice extension for depth view components", "Reserved", "Reserved"};

static void usage()
{
  extern char *__progname;

  (void) fprintf(stderr, "Usage: %s\n", __progname);
  exit(1);
}

static size_t nal_unit_decode(uint8_t *data, size_t size)
{
  uint8_t *b, *e;

  b = data;
  e = data + size;
  while (1)
    {
      b = memmem(b, e - b, (uint8_t[]){0x00, 0x00, 0x03}, 3);
      if (!b)
        break;
      b += 2;
      e --;
      memmove(b, b + 1, e - b);
    }
  return e - data;
}

static void nal_unit_debug(uint8_t *data, size_t size)
{
  uint8_t ref, type;
  size_t i;
  const char *desc;

  ref = (data[0] >> 5) & 0x03;
  type = data[0] & 0x1f;
  desc = type >= 24 ? "Unspecified" : desc_type[type];
  (void) fprintf(stdout, "[NAL] %s (%d/%d)\n", desc, ref, type);
  data ++;
  size --;
  size = nal_unit_decode(data, size);
  for (i = 0; i < size; i ++)
    fprintf(stdout, "%02x", data[i]);
  fprintf(stdout, "\n");
}

static void nal_stream_debug(uint8_t *data, size_t size)
{
  uint8_t *begin = data, *end = begin + size, *next;

  while (1)
    {
      begin = memmem(begin, end - begin, (uint8_t[]){0x00, 0x00, 0x01}, 3);
      if (!begin)
        break;
      begin += 3;
      next = memmem(begin, end - begin, (uint8_t[]){0x00, 0x00, 0x01}, 3);
      if (!next)
        next = end;
      else if (next[-1] == 0x00)
        next --;
      nal_unit_debug(begin, next - begin);
      begin = next;
    }
}

int main(int argc, char **argv)
{
  buffer buffer;
  uint8_t block[1048576];
  ssize_t n;

  (void) argv;
  if (argc != 1)
    usage();

  buffer_construct(&buffer);
  while (1)
    {
      n = read(0, block, sizeof block);
      if (n == -1)
        err(1, "read");
      if (n == 0)
        break;
      buffer_insert(&buffer, buffer_size(&buffer), block, n);
    }

  nal_stream_debug(buffer_data(&buffer), buffer_size(&buffer));
  buffer_destruct(&buffer);
}
