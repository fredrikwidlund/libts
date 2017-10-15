#ifndef TS_STREAM_H_INCLUDED
#define TS_STREAM_H_INCLUDED

typedef struct ts_stream_iterator ts_stream_iterator;
typedef struct ts_stream_pes ts_stream_pes;
typedef struct ts_stream ts_stream;

struct ts_stream_iterator
{
  ts_pes            **current;
  ts_pes            **end;
  ts_stream_pes      *pes_stream;
};

struct ts_stream_pes
{
  int                 pid;
  uint8_t             type;
  ts_pmt_descriptor   descriptor;
  list                packets;
};

struct ts_stream
{
  int                 program_pid;
  int                 pcr_pid;
  ts_pmt_descriptor   descriptor;
  list                streams;
};

void           ts_stream_pes_construct(ts_stream_pes *);
void           ts_stream_pes_destruct(ts_stream_pes *);
void           ts_stream_pes_debug(ts_stream_pes *, FILE *);

void           ts_stream_construct(ts_stream *);
void           ts_stream_destruct(ts_stream *);
ts_stream_pes *ts_stream_lookup(ts_stream *, int);
ts_stream_pes *ts_stream_create(ts_stream *, int);
ssize_t        ts_stream_pack_pat(ts_stream *, ts_units *);
ssize_t        ts_stream_unpack_pat(ts_stream *, ts_units *);
ssize_t        ts_stream_pack_pmt(ts_stream *, ts_units *);
ssize_t        ts_stream_unpack_pmt(ts_stream *, ts_units *);

ssize_t        ts_stream_unpack(ts_stream *, ts_units *);
ssize_t        ts_stream_pack(ts_stream *, ts_units *);
ssize_t        ts_stream_load(ts_stream *, char *);
ssize_t        ts_stream_save(ts_stream *, char *);
void           ts_stream_debug(ts_stream *, FILE *);

#endif /* TS_STREAM_H_INCLUDED */
