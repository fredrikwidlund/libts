#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <dynamic.h>

#include "bytestream.h"
#include "ts_ebp.h"
#include "ts_adaptation_field.h"
#include "ts_packet.h"
#include "ts_packets.h"
#include "ts_unit.h"
#include "ts_units.h"
#include "ts_pat.h"
#include "ts_pmt.h"
#include "ts_pes.h"
#include "ts_stream.h"

static void ts_stream_streams_release(void *object)
{
  ts_stream_pes *pes_stream = *(ts_stream_pes **) object;

  ts_stream_pes_destruct(pes_stream);
  free(pes_stream);
}

static void ts_stream_pes_packets_release(void *object)
{
  ts_pes *pes = *(ts_pes **) object;

  ts_pes_destruct(pes);
  free(pes);
}

void ts_stream_pes_construct(ts_stream_pes *pes_stream)
{
  *pes_stream = (ts_stream_pes) {0};
  list_construct(&pes_stream->packets);
}

void ts_stream_pes_destruct(ts_stream_pes *pes_stream)
{
  list_destruct(&pes_stream->packets, ts_stream_pes_packets_release);
  ts_pmt_descriptor_destruct(&pes_stream->descriptor);
  *pes_stream = (ts_stream_pes) {0};
}

void ts_stream_pes_debug(ts_stream_pes *pes_stream, FILE *f)
{
  ts_pes **p;
  size_t count, size;

  count = 0;
  size = 0;
  list_foreach(&pes_stream->packets, p)
    {
      count ++;
      size += (*p)->size;
    }

  list_foreach(&pes_stream->packets, p)
    ts_pes_debug(*p, f);
}

void ts_stream_construct(ts_stream *ts_stream)
{
  *ts_stream = (struct ts_stream) {0};
  ts_pmt_descriptor_destruct(&ts_stream->descriptor);
  list_construct(&ts_stream->streams);
}

void ts_stream_destruct(ts_stream *ts_stream)
{
  list_destruct(&ts_stream->streams, ts_stream_streams_release);
}

ts_stream_pes *ts_stream_lookup(ts_stream *ts_stream, int pid)
{
  ts_stream_pes **s;

  list_foreach(&ts_stream->streams, s)
    if ((*s)->pid == pid)
      return *s;

  return NULL;
}

ts_stream_pes *ts_stream_create(ts_stream *ts_stream, int pid)
{
  ts_stream_pes *pes_stream;

  pes_stream = ts_stream_lookup(ts_stream, pid);
  if (pes_stream)
    return pes_stream;

  pes_stream = malloc(sizeof *pes_stream);
  if (!pes_stream)
    abort();
  ts_stream_pes_construct(pes_stream);
  pes_stream->pid = pid;
  list_push_back(&ts_stream->streams, &pes_stream, sizeof pes_stream);

  return pes_stream;
}

void ts_stream_delete(ts_stream *ts_stream, int pid)
{
  ts_stream_pes **s;

  list_foreach(&ts_stream->streams, s)
    if ((*s)->pid == pid)
      {
        list_erase(s, ts_stream_streams_release);
        break;
      }
}

ssize_t ts_stream_pack_pat(ts_stream *ts_stream, ts_units *units)
{
  ts_pat pat;
  ts_unit *unit;
  ssize_t n;

  if (!ts_stream->program_pid)
    return -1;

  ts_pat_construct(&pat);
  pat.id = 0;
  pat.id_extension = 1;
  pat.version = 0;
  pat.program_number = 1;
  pat.program_pid = ts_stream->program_pid;

  unit = malloc(sizeof *unit);
  if (!unit)
    abort();
  ts_unit_construct(unit, 0, 0);
  n = ts_pat_pack_buffer(&pat, &unit->data);
  ts_pat_destruct(&pat);
  if (n == -1)
    {
      ts_unit_destruct(unit);
      free(unit);
      return -1;
    }
  list_push_back(ts_units_list(units), &unit, sizeof unit);

  return 1;
}

ssize_t ts_stream_unpack_pat(ts_stream *ts_stream, ts_units *units)
{
  ts_unit **u;
  ts_pat pat;
  ssize_t n;

  list_foreach(ts_units_list(units), u)
    if ((*u)->pid == 0)
      {
        ts_pat_construct(&pat);
        n = ts_pat_unpack_buffer(&pat, &(*u)->data);
        if (n == -1)
          return -1;
        if (!ts_stream->program_pid)
          ts_stream->program_pid = pat.program_pid;
        if (ts_stream->program_pid != pat.program_pid)
          return -1;
        ts_pat_destruct(&pat);
      }

  return 1;
}

ssize_t ts_stream_pack_pmt(ts_stream *ts_stream, ts_units *units)
{
  ts_pmt pmt;
  ts_pmt_stream pmts;
  ts_stream_pes **p;
  ts_unit *unit;
  ssize_t n;

  if (!ts_stream->program_pid)
    return -1;

  ts_pmt_construct(&pmt);
  pmt.id = 0x02;
  pmt.id_extension = 1;
  pmt.version = 0;
  pmt.pcr_pid = ts_stream->pcr_pid ? ts_stream->pcr_pid : 0x1fff;
  ts_pmt_descriptor_copy(&pmt.descriptor, &ts_stream->descriptor);
  list_foreach(&ts_stream->streams, p)
    {
      pmts = (ts_pmt_stream) {0};
      pmts.stream_type = (*p)->type;
      pmts.elementary_pid = (*p)->pid;
      ts_pmt_descriptor_copy(&pmts.descriptor, &(*p)->descriptor);
      list_push_back(&pmt.streams, &pmts, sizeof pmts);
    }

  unit = malloc(sizeof *unit);
  if (!unit)
    abort();
  ts_unit_construct(unit, ts_stream->program_pid, 0);
  n = ts_pmt_pack_buffer(&pmt, &unit->data);
  ts_pmt_destruct(&pmt);
  if (n == -1)
    {
      ts_unit_destruct(unit);
      free(unit);
      return -1;
    }

  list_push_back(ts_units_list(units), &unit, sizeof unit);

  return 1;
}

ssize_t ts_stream_unpack_pmt(ts_stream *ts_stream, ts_units *units)
{
  ts_unit **u;
  ts_pmt pmt;
  ts_pmt_stream *s;
  ts_stream_pes *pes_stream;
  ssize_t n;

  if (!ts_stream->program_pid)
    return -1;

  list_foreach(ts_units_list(units), u)
    if ((*u)->pid == ts_stream->program_pid)
      {
        ts_pmt_construct(&pmt);
        n = ts_pmt_unpack_buffer(&pmt, &(*u)->data);
        if (n == -1)
          {
            ts_pmt_destruct(&pmt);
            return -1;
          }
        if (!ts_stream->pcr_pid)
          ts_stream->pcr_pid = pmt.pcr_pid;
        if (ts_stream->pcr_pid != pmt.pcr_pid)
          {
            ts_pmt_destruct(&pmt);
            return -1;
          }

        list_foreach(&pmt.streams, s)
          {
            pes_stream = ts_stream_create(ts_stream, s->elementary_pid);
            if (!pes_stream->type)
              pes_stream->type = s->stream_type;
            if (pes_stream->type != s->stream_type)
              {
                ts_pmt_destruct(&pmt);
                return -1;
              }
            if (!ts_pmt_descriptor_size(&pes_stream->descriptor) && ts_pmt_descriptor_size(&s->descriptor))
              ts_pmt_descriptor_copy(&pes_stream->descriptor, &s->descriptor);
            if (!ts_pmt_descriptor_equal(&pes_stream->descriptor, &s->descriptor))
              {
                ts_pmt_destruct(&pmt);
                return -1;
              }
          }
        ts_pmt_destruct(&pmt);
      }
  return 1;
}

ssize_t ts_stream_pack_pes(ts_stream *ts_stream, ts_units *units)
{
  vector iterators;
  ts_stream_iterator iterator, *is;
  ts_stream_pes **s, *pes_stream;
  ts_pes *pes;
  ts_unit *unit;
  ssize_t i, i_min, is_count, n, pcr_recorded;

  vector_construct(&iterators, sizeof iterator);
  list_foreach(&ts_stream->streams, s)
    vector_push_back(&iterators, (ts_stream_iterator[]){{
          .current = list_front(&(*s)->packets), .end = list_end(&(*s)->packets), .pes_stream = *s}});

  is = vector_data(&iterators);
  is_count = vector_size(&iterators);
  pcr_recorded = 0;
  while (1)
    {
      i_min = -1;
      for (i = 0; i < is_count; i ++)
        if (is[i].current != is[i].end && (*is[i].current)->pts_indicator &&
            (i_min == -1 || (*is[i].current)->pts < (*is[i_min].current)->pts))
          i_min = i;
      if (i_min == -1)
        break;

      pes = *is[i_min].current;
      pes_stream = is[i_min].pes_stream;
      is[i_min].current = list_next(is[i_min].current);

      unit = malloc(sizeof *unit);
      if (!unit)
        abort();
      ts_unit_construct(unit, pes_stream->pid, pes->random_access_indicator);
      if (ts_stream->pcr_pid == pes_stream->pid && !pcr_recorded)
        {
          unit->pcr_flag = 1;
          unit->pcr = pes->pts * 300;
          pcr_recorded = 1;
        }
      n = ts_pes_pack_buffer(pes, &unit->data);
      if (n == -1)
        {
          ts_unit_destruct(unit);
          free(unit);
          return -1;
        }
      list_push_back(ts_units_list(units), &unit, sizeof unit);
    }

  vector_destruct(&iterators, NULL);

  return 1;
}

ssize_t ts_stream_unpack_pes(ts_stream *ts_stream, ts_units *units)
{
  ts_stream_pes **p;
  ts_unit **u;
  ts_pes *pes;
  ssize_t n, count;

  count = 0;
  list_foreach(&ts_stream->streams, p)
    {
      list_foreach(ts_units_list(units), u)
        {
          if ((*u)->pid == (*p)->pid)
            {
              pes = malloc(sizeof *pes);
              if (!pes)
                abort();
              ts_pes_construct(pes);
              n = ts_pes_unpack_buffer(pes, &(*u)->data);
              if (n == -1)
                {
                  ts_pes_destruct(pes);
                  free(pes);
                }
              else
                {
                  pes->random_access_indicator = (*u)->random_access_indicator;
                  list_push_back(&(*p)->packets, &pes, sizeof pes);
                  count ++;
                }
            }
          else count ++;
        }
    }

  return 1;
}

ssize_t ts_stream_unpack(ts_stream *ts_stream, ts_units *units)
{
  ssize_t n;

  n = ts_stream_unpack_pat(ts_stream, units);
  if (n >= 0)
    n = ts_stream_unpack_pmt(ts_stream, units);
  if (n >= 0)
    n = ts_stream_unpack_pes(ts_stream, units);
  return n;
}

ssize_t ts_stream_pack(ts_stream *ts_stream, ts_units *units)
{
  ssize_t n;

  n = ts_stream_pack_pat(ts_stream, units);
  if (n >= 0)
    n = ts_stream_pack_pmt(ts_stream, units);
  if (n >= 0)
    n = ts_stream_pack_pes(ts_stream, units);
  return n;
}

ssize_t ts_stream_load(ts_stream *ts_stream, char *path)
{
  ts_units units;
  ssize_t n;

  ts_units_construct(&units);
  n = ts_units_load(&units, path);
  if (n == -1)
    {
      ts_units_destruct(&units);
      return -1;
    }

  n = ts_stream_unpack(ts_stream, &units);
  ts_units_destruct(&units);
  return n;
}

ssize_t ts_stream_save(ts_stream *ts_stream, char *path)
{
  ts_units units;
  ssize_t n;

  ts_units_construct(&units);
  n = ts_stream_pack(ts_stream, &units);
  if (n == -1)
    {
      ts_units_destruct(&units);
      return -1;
    }

  n = ts_units_save(&units, path);
  ts_units_destruct(&units);

  return n;
}

void ts_stream_debug(ts_stream *ts_stream, FILE *f)
{
  ts_stream_pes **s;

  (void) fprintf(f, "[stream program pid %d, pcr pid %d]\n", ts_stream->program_pid, ts_stream->pcr_pid);
  list_foreach(&ts_stream->streams, s)
    ts_stream_pes_debug(*s, f);
}
