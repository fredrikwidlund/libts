#ifndef BYTESTREAM_H_INCLUDED
#define BYTESTREAM_H_INCLUDED

typedef struct bytestream bytestream;
struct bytestream
{
  buffer  *buffer;
  uint8_t *data;
  size_t   begin;
  size_t   end;
};

void      bytestream_construct(bytestream *, void *, size_t);
void      bytestream_construct_buffer(bytestream *, buffer *);
void      bytestream_destruct(bytestream *);
int       bytestream_valid(bytestream *);
void     *bytestream_data(bytestream *);
size_t    bytestream_size(bytestream *);

void      bytestream_read(bytestream *, void *, size_t);
uint8_t   bytestream_read8(bytestream *);
uint16_t  bytestream_read16(bytestream *);
uint32_t  bytestream_read32(bytestream *);
uint64_t  bytestream_read64(bytestream *);
uint64_t  bytestream_read_bits(uint64_t, int, int, int);

void      bytestream_write(bytestream *, void *, size_t);
void      bytestream_write8(bytestream *, uint8_t);
void      bytestream_write16(bytestream *, uint16_t);
void      bytestream_write32(bytestream *, uint32_t);
void      bytestream_write64(bytestream *, uint64_t);
uint64_t  bytestream_write_bits(uint64_t, int, int, int);

#endif /* BYTESTREAM_H_INCLUDED */
