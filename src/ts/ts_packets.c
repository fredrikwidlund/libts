#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <dynamic.h>

#include "ts_ebp.h"
#include "ts_adaptation_field.h"
#include "ts_packet.h"
#include "ts_packets.h"

static void ts_packets_list_release(void *object)
{
  ts_packet *p = *(ts_packet **) object;

  ts_packet_destruct(p);
  free(p);
}

void ts_packets_construct(ts_packets *packets)
{
  list_construct(&packets->list);
}

void ts_packets_destruct(ts_packets *packets)
{
  list_destruct(&packets->list, ts_packets_list_release);
}

list *ts_packets_list(ts_packets *packets)
{
  return &packets->list;
}

ssize_t ts_packets_pack(ts_packets *packets, stream *s)
{
  ts_packet **i;
  ssize_t n, count;

  count = 0;
  list_foreach(ts_packets_list(packets), i)
    {
      n = ts_packet_pack(*i, s);
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
      if (n == -1)
        {
          ts_packet_destruct(packet);
          free(packet);
          return -1;
        }
      list_push_back(ts_packets_list(packets), &packet, sizeof packet);
      count ++;
    }

  return stream_valid(s) ? count : -1;
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
