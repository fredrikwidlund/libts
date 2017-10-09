#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <dynamic.h>

#include "ts_ebp.h"

void ts_ebp_construct(ts_ebp *ebp)
{
  *ebp = (ts_ebp) {0};
}

void ts_ebp_destruct(ts_ebp *ebp)
{
    *ebp = (ts_ebp) {0};
}

ssize_t ts_ebp_unpack(ts_ebp *ebp, stream *s)
{
  int tag, flags;
  char id[4];

  tag = stream_read8(s);
  (void) stream_read8(s);
  stream_read(s, id, 4);
  if (tag != 0xdf || strncmp(id, "EBP0", 4))
    return 0;
  ebp->marker = 1;

  flags = stream_read8(s);
  if (flags & 0x01)
    (void) stream_read8(s);
  if (flags & 0x20)
    (void) stream_read8(s);
  if (flags & 0x10)
    (void) stream_read16(s);
  if (flags & 0x08)
    ebp->time = stream_read64(s);

  return stream_valid(s) ? 1 : -1;
}
