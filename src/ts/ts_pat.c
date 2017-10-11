#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <dynamic.h>

#include "ts_psi.h"
#include "ts_pat.h"

void ts_pat_construct(ts_pat *pat)
{
  *pat = (ts_pat) {0};
}

ssize_t ts_pat_construct_buffer(ts_pat *pat, buffer *buffer)
{
  ssize_t n;

  ts_pat_construct(pat);
  n = ts_pat_unpack_buffer(pat, buffer);
  if (n <= 0)
    ts_pat_destruct(pat);
  return n;
}

void ts_pat_destruct(ts_pat *pat)
{
  *pat = (ts_pat) {0};
}

ssize_t ts_pat_pack_stream(ts_pat *pat, stream *s)
{
  ts_psi psi;
  ssize_t n;
  buffer buffer;
  stream pat_stream;

  n = ts_psi_pointer_pack(s);
  if (n == -1)
    return -1;

  buffer_construct(&buffer);
  stream_construct_buffer(&pat_stream, &buffer);
  stream_write32(&pat_stream,
                 stream_write_bits(pat->program_number, 32, 0, 16) |
                 stream_write_bits(0x07, 32, 16, 3) |
                 stream_write_bits(pat->program_pid, 32, 19, 13));
  ts_psi_construct(&psi);
  psi.id = 0x00;
  psi.id_extension = 0x01;
  psi.version = 0;
  psi.current = 1;
  psi.section_number = 0;
  psi.last_section_number = 0;
  n = ts_psi_pack_stream(&psi, s, &pat_stream);
  ts_psi_destruct(&psi);
  stream_destruct(&pat_stream);
  buffer_destruct(&buffer);

  return n;
}

ssize_t ts_pat_pack_buffer(ts_pat *pat, buffer *buffer)
{
  stream stream;
  ssize_t n;

  stream_construct_buffer(&stream, buffer);
  n = ts_pat_pack_stream(pat, &stream);
  stream_destruct(&stream);

  return n;

}

ssize_t ts_pat_unpack_stream(ts_pat *pat, stream *stream)
{
  ts_psi psi;
  ssize_t n;
  int v;

  n = ts_psi_pointer_unpack(stream);
  if (n <= 0)
    return n;

  n = ts_psi_construct_stream(&psi, stream);
  if (n <= 0)
    return n;
  pat->id = psi.id;
  pat->id_extension = psi.id_extension;
  pat->version = psi.version;
  ts_psi_destruct(&psi);
  v = stream_read32(stream);
  if (stream_read_bits(v, 32, 16, 3) != 0x07)
    return -1;
  pat->program_number = stream_read_bits(v, 32, 0, 16);
  pat->program_pid = stream_read_bits(v, 32, 19, 13);
  v = stream_read32(stream);
  fprintf(stderr, "in %08x\n", v);

  if (pat->id != 0x00)
    return -1;

  return stream_valid(stream) ? 1 : -1;
}

ssize_t ts_pat_unpack_buffer(ts_pat *pat, buffer *buffer)
{
  stream stream;
  ssize_t n;

  stream_construct_buffer(&stream, buffer);
  n = ts_pat_unpack_stream(pat, &stream);
  stream_destruct(&stream);

  return n;
}

void ts_pat_debug(ts_pat *pat, FILE *f)
{
  (void) fprintf(f, "[pat] id 0x%02x, id extension 0x%02x, version %d, program number %d, program pid %d\n",
                 pat->id, pat->id_extension, pat->version, pat->program_number, pat->program_pid);
}
