#ifndef TS_STREAM_H_INCLUDED
#define TS_STREAM_H_INCLUDED

typedef struct ts_stream_pes ts_stream_pes;
struct ts_stream_pes
{
  int  pid;
  int  type;
  list packets;
};

typedef struct ts_stream ts_stream;
struct ts_stream
{
  int  program_pid;
  int  pcr_pid;
  list streams;
};

void           ts_stream_pes_construct(ts_stream_pes *);
void           ts_stream_pes_destruct(ts_stream_pes *);

void           ts_stream_construct(ts_stream *);
void           ts_stream_destruct(ts_stream *);
ts_stream_pes *ts_stream_lookup(ts_stream *, int);
ts_stream_pes *ts_stream_create(ts_stream *, int);
ssize_t        ts_stream_load(ts_stream *, char *);
ssize_t        ts_stream_save(ts_stream *, char *);

#endif /* TS_STREAM_H_INCLUDED */
