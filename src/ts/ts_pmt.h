#ifndef TS_PMT_H_INCLUDED
#define TS_PMT_H_INCLUDED

typedef struct ts_pmt_descriptor ts_pmt_descriptor;
struct ts_pmt_descriptor
{
  uint8_t            tag;
  uint8_t            size;
  void              *data;
};

typedef struct ts_pmt_stream ts_pmt_stream;
struct ts_pmt_stream
{
  uint8_t            stream_type;
  unsigned           elementary_pid:13;
  ts_pmt_descriptor  descriptor;
};

typedef struct ts_pmt ts_pmt;
struct ts_pmt
{
  int                id;
  int                id_extension;
  int                version;
  unsigned           pcr_pid:13;
  ts_pmt_descriptor  descriptor;
  list               streams;
};

void    ts_pmt_descriptor_construct(ts_pmt_descriptor *);
void    ts_pmt_descriptor_destruct(ts_pmt_descriptor *);
ssize_t ts_pmt_descriptor_size(ts_pmt_descriptor *);
ssize_t ts_pmt_descriptor_pack(ts_pmt_descriptor *, bytestream *);
ssize_t ts_pmt_descriptor_unpack(ts_pmt_descriptor *, bytestream *, size_t);
int     ts_pmt_descriptor_equal(ts_pmt_descriptor *, ts_pmt_descriptor *);
void    ts_pmt_descriptor_copy(ts_pmt_descriptor *, ts_pmt_descriptor *);

void    ts_pmt_construct(ts_pmt *);
ssize_t ts_pmt_construct_buffer(ts_pmt *, buffer *);
void    ts_pmt_destruct(ts_pmt *);
ssize_t ts_pmt_pack_stream(ts_pmt *, bytestream *);
ssize_t ts_pmt_pack_buffer(ts_pmt *, buffer *);
ssize_t ts_pmt_unpack_stream(ts_pmt *, bytestream *);
ssize_t ts_pmt_unpack_buffer(ts_pmt *, buffer *);
void    ts_pmt_debug(ts_pmt *, FILE *);

#endif /* TS_PMT_H_INCLUDED */
