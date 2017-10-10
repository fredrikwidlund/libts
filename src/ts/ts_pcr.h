#ifndef TS_PCR_H_INCLUDED
#define TS_PCR_H_INCLUDED

ssize_t ts_pcr_pack(uint64_t *, stream *);
ssize_t ts_pcr_unpack(uint64_t *, stream *);

#endif /* TS_PCR_H_INCLUDED */
