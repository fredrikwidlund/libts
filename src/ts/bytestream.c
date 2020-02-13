#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>

#include <dynamic.h>

#include "bytestream.h"

void bytestream_construct(bytestream *bytestream, void *data, size_t size)
{
  bytestream->buffer = NULL;
  bytestream->data = data;
  bytestream->begin = 0;
  bytestream->end = size;
}

void bytestream_construct_buffer(bytestream *bytestream, buffer *buffer)
{
  bytestream->buffer = buffer;
  bytestream->data = buffer_data(bytestream->buffer);
  bytestream->begin = 0;
  bytestream->end = buffer_size(bytestream->buffer);
}

void bytestream_destruct(bytestream *bytestream)
{
  *bytestream = (struct bytestream) {0};
}

int bytestream_valid(bytestream *bytestream)
{
  return bytestream->data != NULL;
}

void *bytestream_data(bytestream *bytestream)
{
  return bytestream->data + bytestream->begin;
}

size_t bytestream_size(bytestream *bytestream)
{
  return bytestream->end - bytestream->begin;
}

void bytestream_read(bytestream *bytestream, void *data, size_t size)
{
  if (size > bytestream_size(bytestream) || !bytestream_valid(bytestream))
    {
      if (data)
        memset(data, 0, size);
      bytestream_destruct(bytestream);
      return;
    }

  if (data)
    memcpy(data, bytestream->data + bytestream->begin, size);
  bytestream->begin += size;
}

uint8_t bytestream_read8(bytestream *bytestream)
{
  uint8_t value;

  bytestream_read(bytestream, &value, sizeof value);
  return value;
}

uint16_t bytestream_read16(bytestream *bytestream)
{
  uint16_t value;

  bytestream_read(bytestream, &value, sizeof value);
  return be16toh(value);
}

uint32_t bytestream_read32(bytestream *bytestream)
{
  uint32_t value;

  bytestream_read(bytestream, &value, sizeof value);
  return be32toh(value);
}

uint64_t bytestream_read64(bytestream *bytestream)
{
  uint64_t value;

  bytestream_read(bytestream, &value, sizeof value);
  return be64toh(value);
}

uint64_t bytestream_read_bits(uint64_t value, int size, int offset, int bits)
{
  value >>= (size - offset - bits);
  value &= (1 << bits) - 1;
  return value;
}

void bytestream_write(bytestream *bytestream, void *data, size_t size)
{
  if (!bytestream->buffer)
    {
      bytestream_destruct(bytestream);
      return;
    }

  buffer_insert(bytestream->buffer, buffer_size(bytestream->buffer), data, size);
  bytestream->data = buffer_data(bytestream->buffer);
  bytestream->end += size;
}

void bytestream_write8(bytestream *bytestream, uint8_t value)
{
  bytestream_write(bytestream, &value, sizeof value);
}

void bytestream_write16(bytestream *bytestream, uint16_t value)
{
  value = htobe16(value);
  bytestream_write(bytestream, &value, sizeof value);
}

void bytestream_write32(bytestream *bytestream, uint32_t value)
{
  value = htobe32(value);
  bytestream_write(bytestream, &value, sizeof value);
}

void bytestream_write64(bytestream *bytestream, uint64_t value)
{
  value = htobe64(value);
  bytestream_write(bytestream, &value, sizeof value);
}

uint64_t bytestream_write_bits(uint64_t value, int size, int offset, int bits)
{
  value &= (1 << bits) - 1;
  value <<= (size - offset - bits);
  return value;
}
