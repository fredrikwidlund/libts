#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <dynamic.h>

#include "ts_psi.h"
#include "ts_pmt.h"

static void ts_pmt_streams_release(void *object)
{
  ts_pmt_stream *pmts = object;

  ts_pmt_descriptor_destruct(&pmts->descriptor);
}

void ts_pmt_descriptor_construct(ts_pmt_descriptor *descriptor)
{
  *descriptor = (ts_pmt_descriptor) {0};
}

void ts_pmt_descriptor_destruct(ts_pmt_descriptor *descriptor)
{
  free(descriptor->data);
  *descriptor = (ts_pmt_descriptor) {0};
}

ssize_t ts_pmt_descriptor_size(ts_pmt_descriptor *descriptor)
{
  return descriptor->tag ? descriptor->size + 2 : 0;
}

ssize_t ts_pmt_descriptor_pack(ts_pmt_descriptor *descriptor, stream *stream)
{
  if (descriptor->tag)
    {
      stream_write8(stream, descriptor->tag);
      stream_write8(stream, descriptor->size);
      stream_write(stream, descriptor->data, descriptor->size);
    }
  return stream_valid(stream) ? 1 : -1;
}

ssize_t ts_pmt_descriptor_unpack(ts_pmt_descriptor *descriptor, stream *stream, size_t len)
{
  if (len < 2 || len > stream_size(stream))
    return -1;
  descriptor->tag = stream_read8(stream);
  descriptor->size = stream_read8(stream);
  if ((size_t) descriptor->size + 2 > len)
    return -1;
  descriptor->data = malloc(descriptor->size);
  if (!descriptor->data)
    abort();
  stream_read(stream, descriptor->data, descriptor->size);
  stream_read(stream, NULL, len - descriptor->size - 2);
  return stream_valid(stream) ? 1 : -1;
}

int ts_pmt_descriptor_equal(ts_pmt_descriptor *a, ts_pmt_descriptor *b)
{
  return
    (a->tag == 0 && b->tag == 0) ||
    (a->tag == b->tag && a->size == b->size && memcmp(a->data, b->data, a->size) == 0);
}

void ts_pmt_descriptor_copy(ts_pmt_descriptor *dst, ts_pmt_descriptor *src)
{
  dst->tag = src->tag;
  dst->size = src->size;
  if (dst->size)
    {
      dst->data = malloc(dst->size);
      if (!dst->data)
        abort();
      memcpy(dst->data, src->data, dst->size);
    }
}

void ts_pmt_construct(ts_pmt *pmt)
{
  *pmt = (ts_pmt) {0};
  ts_pmt_descriptor_construct(&pmt->descriptor);
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
  ts_pmt_descriptor_destruct(&pmt->descriptor);
  list_destruct(&pmt->streams, ts_pmt_streams_release);
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
                 stream_write_bits(ts_pmt_descriptor_size(&pmt->descriptor), 16, 6, 10));
  if (ts_pmt_descriptor_size(&pmt->descriptor))
    (void) ts_pmt_descriptor_pack(&pmt->descriptor, &pmt_stream);
  list_foreach(&pmt->streams, pmts)
    {
      stream_write8(&pmt_stream, pmts->stream_type);
      stream_write16(&pmt_stream,
                     stream_write_bits(0x07, 16, 0, 3) |
                     stream_write_bits(pmts->elementary_pid, 16, 3, 13));
      stream_write16(&pmt_stream,
                     stream_write_bits(0x0f, 16, 0, 4) |
                     stream_write_bits(0, 16, 4, 2) |
                     stream_write_bits(ts_pmt_descriptor_size(&pmts->descriptor), 16, 6, 10));
      if (ts_pmt_descriptor_size(&pmts->descriptor))
        (void) ts_pmt_descriptor_pack(&pmts->descriptor, &pmt_stream);
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
  size_t size, len;
  int v, valid;
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
  if (len)
    {
      n = ts_pmt_descriptor_unpack(&pmt->descriptor, &data, len);
      if (n == -1)
        {
          stream_destruct(&data);
          return -1;
        }
    }

  while (stream_size(&data))
    {
      pmt_stream = (ts_pmt_stream){0};
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
      if (len)
        {
          n = ts_pmt_descriptor_unpack(&pmt_stream.descriptor, &data, len);
          if (n == -1)
            {
              stream_destruct(&data);
              return -1;
            }
        }
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
