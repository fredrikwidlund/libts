#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <dynamic.h>

#include "bytestream.h"
#include "ts_ebp.h"

void ts_ebp_construct(ts_ebp *ebp)
{
  *ebp = (ts_ebp) {0};
}

void ts_ebp_destruct(ts_ebp *ebp)
{
    *ebp = (ts_ebp) {0};
}

ssize_t ts_ebp_unpack(ts_ebp *ebp, bytestream *s)
{
  int tag, flags;
  char id[4];

  tag = bytestream_read8(s);
  (void) bytestream_read8(s);
  bytestream_read(s, id, 4);
  if (tag != 0xdf || strncmp(id, "EBP0", 4))
    return 0;
  ebp->marker = 1;

  flags = bytestream_read8(s);
  if (flags & 0x01)
    (void) bytestream_read8(s);
  if (flags & 0x20)
    (void) bytestream_read8(s);
  if (flags & 0x10)
    (void) bytestream_read16(s);
  if (flags & 0x08)
    ebp->time = bytestream_read64(s);

  return bytestream_valid(s) ? 1 : -1;
}
