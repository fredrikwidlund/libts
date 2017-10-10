#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <dynamic.h>

ssize_t ts_pcr_pack(uint64_t *value, stream *stream)
{
  uint64_t base, ext, v;

  base = *value / 300;
  ext = *value - (base * 300);
  v = (base << 15) + (ext & 0x1ff);
  stream_write32(stream, v >> 16);
  stream_write16(stream, v);

  return stream_valid(stream) ? 1 : -1;
}

ssize_t ts_pcr_unpack(uint64_t *value, stream *stream)
{
  uint64_t v;

  v = ((uint64_t) stream_read32(stream) << 16) + stream_read16(stream);
  *value = (stream_read_bits(v, 48, 0, 33) * 300) + stream_read_bits(v, 48, 39, 9);

  return stream_valid(stream) ? 1 : -1;
}
