#ifndef TS_UNITS_H_INCLUDED
#define TS_UNITS_H_INCLUDED

typedef struct ts_units ts_units;
struct ts_units
{
  list list;
};

void     ts_units_construct(ts_units *);
void     ts_units_destruct(ts_units *);
list    *ts_units_list(ts_units *);
ssize_t  ts_units_pack(ts_units *, ts_packets *);
ssize_t  ts_units_unpack(ts_units *, ts_packets *);
ssize_t  ts_units_load(ts_units *, char *);
ssize_t  ts_units_save(ts_units *, char *);

#endif /* TS_UNITS_H_INCLUDED */
