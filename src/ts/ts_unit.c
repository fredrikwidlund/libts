#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <dynamic.h>

#include "ts_ebp.h"
#include "ts_adaptation_field.h"
#include "ts_packet.h"
#include "ts_unit.h"

void ts_unit_construct(ts_unit *unit, int pid, int random_access_indicator)
{
  *unit = (ts_unit) {.pid = pid, .random_access_indicator = random_access_indicator};
  buffer_construct(&unit->data);
}

void ts_unit_destruct(ts_unit *unit)
{
  buffer_destruct(&unit->data);
  *unit = (ts_unit) {0};
}

ssize_t ts_unit_pack(ts_unit *unit, ts_packet *packet)
{
  buffer_insert(&unit->data, buffer_size(&unit->data), packet->payload_data, packet->payload_size);
  return 1;
}


/*
ssize_t ts_unit_pack(ts_unit *unit, stream *stream)
{
  ssize_t n;

  if (!unit->payload_size)
    unit->adaptation_field_control &= ~0x01;

  if (ts_unit_size(unit) < 188)
    unit->adaptation_field_control |= 0x02;

  stream_write32(stream,
                 stream_write_bits(0x47, 32, 0, 8) |
                 stream_write_bits(unit->transport_error_indicator, 32, 8, 1) |
                 stream_write_bits(unit->payload_unit_start_indicator, 32, 9, 1) |
                 stream_write_bits(unit->transport_priority, 32, 10, 1) |
                 stream_write_bits(unit->pid, 32, 11, 13) |
                 stream_write_bits(unit->transport_scrambling_control, 32, 24, 2) |
                 stream_write_bits(unit->adaptation_field_control, 32, 26, 2) |
                 stream_write_bits(unit->continuity_counter, 32, 28, 4));

  if (unit->adaptation_field_control & 0x02)
    {
      n = ts_adaptation_field_pack(&unit->adaptation_field, stream, 188 - 4 - unit->payload_size);
      if (n == -1)
        return -1;
    }

  if (unit->adaptation_field_control & 0x01)
    stream_write(stream, unit->payload_data, unit->payload_size);

  return stream_valid(stream) ? 1 : -1;
}
*/
 /*
ssize_t ts_unit_unpack(ts_unit *unit, ts_packets *packets)
{
  ts_packet **p, *packet;
  list *list;

  list = ts_packets_list(packets);

  list_foreach(list, p)
    if ((*p)->payload_unit_start_indicator)
      break;

  if (p == list_end(list))
    return 0;

  unit->pid = (*p)->pid;
  unit->random_access_indicator = (*p)->adaptation_field.random_access_indicator;
  while (1)
    {
      buffer_insert(&unit->data, buffer_size(&unit->data), (*p)->data, (*p)->size);
      for (p = list_next(p); p != list_end(list); (*p)->pid == unit->pid && !(*p)->payload_unit_start_indicator)
        ;
      
      while (1)
        {
          p = list_next(p);
          if ((*p)->pid == unit->pid && !(*p)->payload_unit_start_indicator)
            break;
        }
    }
  
  p = list_front(list);
  while (p != list_end(list))
    {
      packet = *p;
      if (packet->payload_unit_start_indicator)
        {
          
        }
        {
          ts_packet_destruct(packet);
          free(packet);
          list_erase(p);
        }
    }
 
  ts_unit *unit;

  unit = list_empty(&s->units) ? NULL : *(ts_unit **) list_back(&s->units);
  if (packet->payload_unit_start_indicator)
    {
      unit = malloc(sizeof *unit);
      if (!unit)
        abort();
      ts_unit_construct(unit);
      if (packet->adaptation_field_control & 0x02 &&
          packet->adaptation_field.random_access_indicator)
        unit->random_access_indicator = 1;
      list_push_back(&s->units, &unit, sizeof unit);
    }

  if (!unit)
    return 0;

  ts_unit_append(unit, packet->payload_data, packet->payload_size);
  return 0;
}

  
  size_t size;
  ssize_t n;
  uint32_t v;

  size = stream_size(stream);
  if (size < 188)
    return 0;

  v = stream_read32(stream);
  unit->transport_error_indicator = stream_read_bits(v, 32, 8, 1);
  unit->payload_unit_start_indicator = stream_read_bits(v, 32, 9, 1);
  unit->transport_priority = stream_read_bits(v, 32, 10, 1);
  unit->pid = stream_read_bits(v, 32, 11, 13);
  unit->transport_scrambling_control = stream_read_bits(v, 32, 24, 2);
  unit->adaptation_field_control = stream_read_bits(v, 32, 26, 2);
  unit->continuity_counter = stream_read_bits(v, 32, 28, 4);

  if (unit->adaptation_field_control & 0x02)
    {
      n = ts_adaptation_field_unpack(&unit->adaptation_field, stream);
      if (n <= 0)
        return n;
    }

  if (unit->adaptation_field_control & 0x01)
    {
      unit->payload_size = 188 - (size - stream_size(stream));
      unit->payload_data = malloc(unit->payload_size);
      if (!unit->payload_data)
        abort();
      stream_read(stream, unit->payload_data, unit->payload_size);
    }

  return stream_valid(stream) ? 1 : -1;
}

*/
