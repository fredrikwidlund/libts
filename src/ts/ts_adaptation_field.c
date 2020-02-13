#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <dynamic.h>

#include "bytestream.h"
#include "ts_pcr.h"
#include "ts_ebp.h"
#include "ts_adaptation_field.h"

void ts_adaptation_field_construct(ts_adaptation_field *af)
{
  *af = (ts_adaptation_field) {0};
  ts_ebp_construct(&af->ebp);
}

void ts_adaptation_field_destruct(ts_adaptation_field *af)
{
  ts_ebp_destruct(&af->ebp);
  free(af->private_data);
  ts_adaptation_field_construct(af);
}

size_t ts_adaptation_field_size(ts_adaptation_field *af)
{
  return 2 + (af->pcr_flag ? 6 : 0) + (af->opcr_flag ? 6 : 0) + (af->splicing_point_flag ? 1 : 0) +
    (af->transport_private_data_flag ? af->private_size + 1 : 0);
}

ssize_t ts_adaptation_field_pack(ts_adaptation_field *af, bytestream *stream, size_t size)
{
  bytestream_write8(stream, size - 1);
  if (size == 1)
    return bytestream_valid(stream) ? 1 : -1;

  bytestream_write8(stream,
                bytestream_write_bits(af->discontinuity_indicator, 8, 0, 1) |
                bytestream_write_bits(af->random_access_indicator, 8, 1, 1) |
                bytestream_write_bits(af->elementary_stream_priority_indicator, 8, 2, 1) |
                bytestream_write_bits(af->pcr_flag, 8, 3, 1) |
                bytestream_write_bits(af->opcr_flag, 8, 4, 1) |
                bytestream_write_bits(af->splicing_point_flag, 8, 5, 1) |
                bytestream_write_bits(af->transport_private_data_flag, 8, 6, 1) |
                bytestream_write_bits(af->adaptation_field_extension_flag, 8, 7, 1));

  if (af->pcr_flag)
    ts_pcr_pack(&af->pcr, stream);

  if (af->opcr_flag)
    ts_pcr_pack(&af->opcr, stream);

  if (af->splicing_point_flag)
    bytestream_write8(stream, af->splice_countdown);

  if (af->private_size)
    {
      bytestream_write8(stream, af->private_size);
      bytestream_write(stream, af->private_data, af->private_size);
    }

  size -= ts_adaptation_field_size(af);
  while (size)
    {
      bytestream_write8(stream, 0xff);
      size --;
    }

  return bytestream_valid(stream) ? 1 : -1;
}

ssize_t ts_adaptation_field_unpack(ts_adaptation_field *af, bytestream *s)
{
  size_t size;
  uint8_t len, flags;
  bytestream ebp;

  len = bytestream_read8(s);
  if (len)
    {
      size = bytestream_size(s);
      flags = bytestream_read8(s);
      af->discontinuity_indicator = bytestream_read_bits(flags, 8, 0, 1);
      af->random_access_indicator = bytestream_read_bits(flags, 8, 1, 1);
      af->elementary_stream_priority_indicator = bytestream_read_bits(flags, 8, 2, 1);
      af->pcr_flag = bytestream_read_bits(flags, 8, 3, 1);
      af->opcr_flag = bytestream_read_bits(flags, 8, 4, 1);
      af->splicing_point_flag = bytestream_read_bits(flags, 8, 5, 1);
      af->transport_private_data_flag = bytestream_read_bits(flags, 8, 6, 1);
      af->adaptation_field_extension_flag = bytestream_read_bits(flags, 8, 7, 1);

      if (af->pcr_flag)
        ts_pcr_unpack(&af->pcr, s);

      if (af->opcr_flag)
        ts_pcr_unpack(&af->opcr, s);

      if (af->splicing_point_flag)
        af->splice_countdown = bytestream_read8(s);

      if (af->transport_private_data_flag)
        {
          af->private_size = bytestream_read8(s);
          if (af->private_size > bytestream_size(s))
            return -1;
          af->private_data = malloc(af->private_size);
          if (!af->private_data)
            abort();
          bytestream_read(s, af->private_data, af->private_size);

          bytestream_construct(&ebp, af->private_data, af->private_size);
          (void) ts_ebp_unpack(&af->ebp, &ebp);
          bytestream_destruct(&ebp);
        }

      // extensions not supported yet
      af->adaptation_field_extension_flag = 0;
      bytestream_read(s, NULL, len - (size - bytestream_size(s)));
    }

  return bytestream_valid(s) ? 1 : -1;
}
