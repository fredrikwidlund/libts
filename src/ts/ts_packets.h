#ifndef TS_PACKETS_H_INCLUDED
#define TS_PACKETS_H_INCLUDED

typedef struct ts_packets ts_packets;
struct ts_packets
{
  list list;
};

void     ts_packets_construct(ts_packets *);
void     ts_packets_destruct(ts_packets *);
list    *ts_packets_list(ts_packets *);
ssize_t  ts_packets_pack(ts_packets *, stream *);
ssize_t  ts_packets_unpack(ts_packets *, stream *);
ssize_t  ts_packets_load(ts_packets *, char *);
ssize_t  ts_packets_save(ts_packets *, char *);

#endif /* TS_PACKETS_H_INCLUDED */
