#ifndef TS_EBP_H_INCLUDED
#define TS_EBP_H_INCLUDED

typedef struct ts_ebp ts_ebp;

struct ts_ebp
{
  unsigned  marker:1;
  uint64_t  time;
};

void    ts_ebp_construct(ts_ebp *);
void    ts_ebp_destruct(ts_ebp *);
ssize_t ts_ebp_unpack(ts_ebp *, bytestream *);

#endif /* TS_EBP_H_INCLUDED */

