#ifndef TS_UNIT_H_INCLUDED
#define TS_UNIT_H_INCLUDED

typedef struct ts_unit ts_unit;

struct ts_unit
{
  unsigned pid:13;
  unsigned random_access_indicator:1;
  unsigned pcr_flag:1;
  uint64_t pcr;
  buffer   data;
};

void    ts_unit_construct(ts_unit *, int, int);
void    ts_unit_destruct(ts_unit *);
ssize_t ts_unit_pack(ts_unit *, ts_packets *);
ssize_t ts_unit_unpack(ts_unit *, ts_packet *);
void    ts_unit_compact(ts_unit *);
void    ts_unit_debug(ts_unit *, FILE *);

#endif /* TS_UNIT_H_INCLUDED */
