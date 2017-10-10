#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>

#include <dynamic.h>

#include "ts_ebp.h"
#include "ts_adaptation_field.h"
#include "ts_packet.h"
#include "ts_packets.h"
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

ssize_t ts_unit_pack(ts_unit *unit, ts_packets *packets)
{
  char *base;
  size_t size, count;
  ts_packet *packet;

  count = 0;
  base = buffer_data(&unit->data);
  size = buffer_size(&unit->data);
  while (size)
    {
      packet = malloc(sizeof *packet);
      if (!packet)
        abort();
      ts_packet_construct(packet);
      packet->pid = unit->pid;
      packet->adaptation_field_control |= 1;
      if (count == 0)
        {
          packet->payload_unit_start_indicator = 1;
          if (unit->random_access_indicator)
            {
              packet->adaptation_field.random_access_indicator = 1;
              packet->adaptation_field_control |= 2;
            }
          if (unit->pcr_flag)
            {
              packet->adaptation_field.pcr_flag = 1;
              packet->adaptation_field.pcr = unit->pcr;
              packet->adaptation_field_control |= 2;
            }
        }
      packet->payload_size = MIN(188 - ts_packet_size(packet), size);
      packet->payload_data = malloc(packet->payload_size);
      if (!packet->payload_data)
        abort();
      memcpy(packet->payload_data, base, packet->payload_size);
      list_push_back(ts_packets_list(packets), &packet, sizeof packet);
      base += packet->payload_size;
      size -= packet->payload_size;
      count ++;
    }

  return 0;
}

ssize_t ts_unit_unpack(ts_unit *unit, ts_packet *packet)
{
  buffer_insert(&unit->data, buffer_size(&unit->data), packet->payload_data, packet->payload_size);
  return 0;
}
