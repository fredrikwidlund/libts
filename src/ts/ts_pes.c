#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <dynamic.h>

#include "ts_pes.h"

static ssize_t ts_pes_ts_pack(uint64_t *t, stream *s, int pts_indicator, int dts_indicator)
{
  stream_write8(s,
                stream_write_bits(pts_indicator, 8, 2, 1) |
                stream_write_bits(dts_indicator, 8, 3, 1) |
                stream_write_bits(*t >> 30, 8, 4, 3) |
                stream_write_bits(1, 8, 7, 1));
  stream_write32(s,
                 stream_write_bits(*t >> 15, 32, 0, 15) |
                 stream_write_bits(1, 32, 15, 1) |
                 stream_write_bits(*t, 32, 16, 15) |
                 stream_write_bits(1, 32, 31, 1));
  return stream_valid(s) ? 1 : -1;
}

static ssize_t ts_pes_ts_unpack(uint64_t *t, stream *s)
{
  uint32_t v;

  v = stream_read8(s);
  *t = stream_read_bits(v, 8, 4, 3);

  v = stream_read16(s);
  *t <<= 15;
  *t += stream_read_bits(v, 16, 0, 15);

  v = stream_read16(s);
  *t <<= 15;
  *t += stream_read_bits(v, 16, 0, 15);

  return stream_valid(s) ? 1 : -1;
}

void ts_pes_construct(ts_pes *pes)
{
  *pes = (ts_pes) {0};
}

ssize_t ts_pes_construct_buffer(ts_pes *pes, buffer *buffer)
{
  ssize_t n;

  ts_pes_construct(pes);
  n = ts_pes_unpack_buffer(pes, buffer);
  if (n <= 0)
    ts_pes_destruct(pes);
  return n;
}

void ts_pes_destruct(ts_pes *pes)
{
  free(pes->data);
  *pes = (ts_pes) {0};
}

static size_t ts_pes_header_size(ts_pes *pes)
{
  return 3 + (pes->pts_indicator ? 5 : 0) + (pes->dts_indicator ? 5 : 0);
}

static ssize_t ts_pes_header_pack_stream(ts_pes *pes, stream *s)
{
  stream_write16(s,
                 stream_write_bits(0x02, 16, 0, 2) |
                 stream_write_bits(pes->data_alignment_indicator, 16, 5, 1) |
                 stream_write_bits(pes->pts_indicator, 16, 8, 1) |
                 stream_write_bits(pes->dts_indicator, 16, 9, 1));
  stream_write8(s, ts_pes_header_size(pes) - 3);
  if (pes->pts_indicator)
    ts_pes_ts_pack(&pes->pts, s, pes->pts_indicator, pes->dts_indicator);
  if (pes->dts_indicator)
    ts_pes_ts_pack(&pes->dts, s, 0, pes->dts_indicator);
  return stream_valid(s) ? 1 : -1;
}

ssize_t ts_pes_pack_stream(ts_pes *pes, stream *s)
{
  int len;
  ssize_t n;

  stream_write32(s,
                 stream_write_bits(1, 32, 23, 1) |
                 stream_write_bits(pes->stream_id, 32, 24, 8));
  len = ts_pes_header_size(pes) + pes->size;
  if (pes->stream_id >= 0xe0 && pes->stream_id < 0xf0 && len >= 65536)
    len = 0;
  if (len >= 65536)
    return -1;
  stream_write16(s, len);
  if (pes->stream_id != 0xbe && pes->stream_id != 0xbf)
    {
      n = ts_pes_header_pack_stream(pes, s);
      if (n == -1)
        return -1;
    }
  stream_write(s, pes->data, pes->size);
  return stream_valid(s) ? 1 : -1;
}

ssize_t ts_pes_unpack_stream(ts_pes *pes, stream *s)
{
  int v;
  size_t len;
  ssize_t n;

  v = stream_read32(s);
  if (stream_read_bits(v, 32, 0, 24) != 0x000001)
    return -1;
  pes->stream_id = stream_read_bits(v, 32, 24, 8);
  len = stream_read16(s);
  if (((pes->stream_id < 0xe0 || pes->stream_id >= 0xf0) && len == 0) ||
      (len && len != stream_size(s)))
    return -1;

  if (pes->stream_id != 0xbe && pes->stream_id != 0xbf)
    {
      v = stream_read16(s);
      if (stream_read_bits(v, 16, 0, 2) != 0x02)
        return -1;

      pes->scrambling_control = stream_read_bits(v, 16, 2, 2);
      pes->priority = stream_read_bits(v, 16, 4, 1);
      pes->data_alignment_indicator = stream_read_bits(v, 16, 5, 1);
      pes->copyright = stream_read_bits(v, 16, 6, 1);
      pes->original_or_copy = stream_read_bits(v, 16, 7, 1);
      pes->pts_indicator = stream_read_bits(v, 16, 8, 1);
      pes->dts_indicator = stream_read_bits(v, 16, 9, 1);
      len = stream_read8(s);
      if (pes->pts_indicator)
        {
          n = ts_pes_ts_unpack(&pes->pts, s);
          if (n == -1 || len < 5 || !stream_valid(s))
            return -1;
          len -= 5;
        }
      if (pes->dts_indicator)
        {
          n = ts_pes_ts_unpack(&pes->dts, s);
          if (n == -1 || len < 5 || !stream_valid(s))
            return -1;
          len -= 5;
        }

      stream_read(s, NULL, len);
    }

  if (!stream_valid(s))
    return -1;

  pes->size = stream_size(s);
  pes->data = malloc(pes->size);
  if (!pes->data)
    abort();
  memcpy(pes->data, stream_data(s), pes->size);

  return 1;
}

ssize_t ts_pes_pack_buffer(ts_pes *pes, buffer *buffer)
{
  stream stream;
  ssize_t n;

  stream_construct_buffer(&stream, buffer);
  n = ts_pes_pack_stream(pes, &stream);
  stream_destruct(&stream);
  return n;
}

ssize_t ts_pes_unpack_buffer(ts_pes *pes, buffer *buffer)
{
  stream stream;
  ssize_t n;

  stream_construct_buffer(&stream, buffer);
  n = ts_pes_unpack_stream(pes, &stream);
  stream_destruct(&stream);

  return n;
}

void ts_pes_debug(ts_pes *pes, FILE *f)
{
  (void) fprintf(f, "[pes] stream id 0x%02x, size %lu",
                 pes->stream_id, pes->size);
  if (pes->pts_indicator)
    (void) fprintf(f, ", pts %lu", pes->pts);
  if (pes->pts_indicator)
    (void) fprintf(f, ", dts %lu", pes->dts);
  (void) fprintf(f, "\n");
  }
