#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <dynamic.h>

#include "ts_ebp.h"
#include "ts_adaptation_field.h"
#include "ts_packet.h"
#include "ts_packets.h"

static ts_packets_counter *ts_packets_lookup_counter(ts_packets *packets, int pid)
{
  ts_packets_counter *counters;
  size_t i, size;

  counters = vector_data(&packets->counters);
  size = vector_size(&packets->counters);
  for (i = 0; i < size; i ++)
    if (counters[i].pid == pid)
      return &counters[i];
  vector_push_back(&packets->counters, (ts_packets_counter[]){{.pid = pid}});
  return vector_at(&packets->counters, i);
}

static void ts_packets_list_release(void *object)
{
  ts_packet *p = *(ts_packet **) object;

  ts_packet_destruct(p);
  free(p);
}

void ts_packets_construct(ts_packets *packets)
{
  list_construct(&packets->list);
  vector_construct(&packets->counters, sizeof (struct ts_packets_counter));
}

void ts_packets_destruct(ts_packets *packets)
{
  list_destruct(&packets->list, ts_packets_list_release);
  vector_destruct(&packets->counters);
}

list *ts_packets_list(ts_packets *packets)
{
  return &packets->list;
}

ssize_t ts_packets_pack(ts_packets *packets, stream *s)
{
  ts_packet **p;
  ssize_t n, count;
  ts_packets_counter *counter;

  count = 0;
  list_foreach(ts_packets_list(packets), p)
    {
      counter = ts_packets_lookup_counter(packets, (*p)->pid);
      (*p)->continuity_counter = counter->value;
      if ((*p)->adaptation_field_control & 0x01)
        counter->value ++;
      n = ts_packet_pack(*p, s);
      if (n == -1)
        return -1;
      count ++;
    }

  return count;
}

ssize_t ts_packets_unpack(ts_packets *packets, stream *s)
{
  ts_packet *packet;
  ssize_t n, count;

  count = 0;
  while (stream_size(s))
    {
      packet = malloc(sizeof *packet);
      if (!packet)
        abort();
      ts_packet_construct(packet);
      n = ts_packet_unpack(packet, s);
      if (n <= 0)
        {
          ts_packet_destruct(packet);
          free(packet);
          break;
        }
      list_push_back(ts_packets_list(packets), &packet, sizeof packet);
      count ++;
    }

  return stream_valid(s) ? count : -1;
}

void ts_packets_append(ts_packets *packets, ts_packets *append)
{
  ts_packet **i;

  list_foreach(ts_packets_list(append), i)
    list_push_back(ts_packets_list(packets), i, sizeof *i);
  list_clear(ts_packets_list(append), NULL);
}

ssize_t ts_packets_load(ts_packets *packets, char *path)
{
  buffer b;
  stream s;
  int e;
  ssize_t n;

  buffer_construct(&b);
  e = buffer_load(&b, path);
  if (e == -1)
    {
      buffer_destruct(&b);
      return -1;
    }
  stream_construct_buffer(&s, &b);
  n = ts_packets_unpack(packets, &s);
  stream_destruct(&s);
  buffer_destruct(&b);

  return n;
}

ssize_t ts_packets_save(ts_packets *packets, char *path)
{
  buffer b;
  stream s;
  ssize_t n;

  buffer_construct(&b);
  stream_construct_buffer(&s, &b);
  n = ts_packets_pack(packets, &s);
  if (n >= 0)
    buffer_save(&b, path);
  stream_destruct(&s);
  buffer_destruct(&b);

  return n;
}
