#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <dynamic.h>

#include "ts_psi.h"
#include "ts_pmt.h"

void ts_pmt_construct(ts_pmt *pmt)
{
  *pmt = (ts_pmt) {0};
  list_construct(&pmt->streams);
}

ssize_t ts_pmt_construct_buffer(ts_pmt *pmt, buffer *buffer)
{
  ssize_t n;

  ts_pmt_construct(pmt);
  n = ts_pmt_unpack_buffer(pmt, buffer);
  if (n <= 0)
    ts_pmt_destruct(pmt);
  return n;
}

void ts_pmt_destruct(ts_pmt *pmt)
{
  list_destruct(&pmt->streams, NULL);
  *pmt = (ts_pmt) {0};
}

ssize_t ts_pmt_pack_stream(ts_pmt *pmt, stream *s)
{
  ts_pmt_stream *pmts;
  ts_psi psi;
  ssize_t n;
  buffer buffer;
  stream pmt_stream;

  n = ts_psi_pointer_pack(s);
  if (n == -1)
    return -1;

  buffer_construct(&buffer);
  stream_construct_buffer(&pmt_stream, &buffer);
  stream_write16(&pmt_stream,
                 stream_write_bits(0x07, 16, 0, 3) |
                 stream_write_bits(pmt->pcr_pid, 16, 3, 13));
  stream_write16(&pmt_stream,
                 stream_write_bits(0x0f, 16, 0, 4) |
                 stream_write_bits(0, 16, 4, 2) |
                 stream_write_bits(0, 16, 6, 10));
  list_foreach(&pmt->streams, pmts)
    {
      stream_write8(&pmt_stream, pmts->stream_type);
      stream_write16(&pmt_stream,
                     stream_write_bits(0x07, 16, 0, 3) |
                     stream_write_bits(pmts->elementary_pid, 16, 3, 13));
      stream_write16(&pmt_stream,
                     stream_write_bits(0x0f, 16, 0, 4) |
                     stream_write_bits(0, 16, 4, 2) |
                     stream_write_bits(0, 16, 6, 10));
    }

  ts_psi_construct(&psi);
  psi.id = 0x02;
  psi.id_extension = 0x01;
  psi.version = 0;
  psi.current = 1;
  psi.section_number = 0;
  psi.last_section_number = 0;
  n = ts_psi_pack_stream(&psi, s, &pmt_stream);
  ts_psi_destruct(&psi);
  stream_destruct(&pmt_stream);
  buffer_destruct(&buffer);

  return n;
}

ssize_t ts_pmt_pack_buffer(ts_pmt *pmt, buffer *buffer)
{
  stream stream;
  ssize_t n;

  stream_construct_buffer(&stream, buffer);
  n = ts_pmt_pack_stream(pmt, &stream);
  stream_destruct(&stream);

  return n;
}

ssize_t ts_pmt_unpack_stream(ts_pmt *pmt, stream *s)
{
  ts_psi psi;
  ts_pmt_stream pmt_stream;
  ssize_t n;
  size_t size;
  int v, len, valid;
  stream data;

  n = ts_psi_pointer_unpack(s);
  if (n <= 0)
    return n;

  n = ts_psi_construct_stream(&psi, s);
  if (n <= 0)
    return n;

  pmt->id = psi.id;
  pmt->id_extension = psi.id_extension;
  pmt->version = psi.version;
  size = psi.section_length;
  ts_psi_destruct(&psi);

  // remove table syntax section length (5) and crc (4)
  if (size < 9 || stream_size(s) + 5 < size)
    return -1;
  size -= 9;

  stream_construct(&data, stream_data(s), size);
  v = stream_read32(&data);
  if (stream_read_bits(v, 32, 0, 3) != 0x07 ||
      stream_read_bits(v, 32, 16, 4) != 0x0f ||
      stream_read_bits(v, 32, 20, 2) != 0x00)
    {
      stream_destruct(&data);
      return -1;
    }
  pmt->pcr_pid = stream_read_bits(v, 32, 3, 13);
  len = stream_read_bits(v, 32, 22, 10);
  stream_read(&data, NULL, len);

  while (stream_size(&data))
    {
      pmt_stream.stream_type = stream_read8(&data);
      v = stream_read32(&data);
      if (stream_read_bits(v, 32, 0, 3) != 0x07 ||
          stream_read_bits(v, 32, 16, 4) != 0x0f ||
          stream_read_bits(v, 32, 20, 2) != 0x00)
        {
          stream_destruct(&data);
          return -1;
        }
      pmt_stream.elementary_pid = stream_read_bits(v, 32, 3, 13);
      len = stream_read_bits(v, 32, 22, 10);
      stream_read(&data, NULL, len);
      list_push_back(&pmt->streams, &pmt_stream, sizeof pmt_stream);
    }

  valid = stream_valid(&data);
  stream_destruct(&data);
  if (!valid || pmt->id != 0x02)
    return -1;

  return 1;
}

ssize_t ts_pmt_unpack_buffer(ts_pmt *pmt, buffer *buffer)
{
  stream stream;
  ssize_t n;

  stream_construct_buffer(&stream, buffer);
  n = ts_pmt_unpack_stream(pmt, &stream);
  stream_destruct(&stream);

  return n;
}

void ts_pmt_debug(ts_pmt *pmt, FILE *f)
{
  ts_pmt_stream *i;

  (void) fprintf(f, "[pmt] id 0x%02x, id extension 0x%02x, version %d, pcr pid %d\n",
                 pmt->id, pmt->id_extension, pmt->version, pmt->pcr_pid);
  list_foreach(&pmt->streams, i)
    (void) fprintf(f, "[pmt stream] stream_type %d, elementary pid %d\n", i->stream_type, i->elementary_pid);
}
