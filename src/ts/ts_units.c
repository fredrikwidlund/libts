#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <dynamic.h>

#include "bytestream.h"
#include "ts_ebp.h"
#include "ts_adaptation_field.h"
#include "ts_packet.h"
#include "ts_packets.h"
#include "ts_unit.h"
#include "ts_units.h"

static void ts_units_list_release(void *object)
{
  ts_unit *u = *(ts_unit **) object;

  ts_unit_destruct(u);
  free(u);
}

void ts_units_construct(ts_units *units)
{
  list_construct(&units->list);
}

void ts_units_destruct(ts_units *units)
{
  list_destruct(&units->list, ts_units_list_release);
}

list *ts_units_list(ts_units *units)
{
  return &units->list;
}

ssize_t ts_units_pack(ts_units *units, ts_packets *packets)
{
  ts_unit **u;
  ssize_t n, count;

  count = 0;
  list_foreach(ts_units_list(units), u)
    {
      n = ts_unit_pack(*u, packets);
      if (n == -1)
        return -1;
      count ++;
    }

  return count;
}

ssize_t ts_units_unpack(ts_units *units, ts_packets *packets)
{
  list incomplete;
  ts_packet **p;
  ts_unit **u, **u_found, *unit;
  ssize_t count;

  count = 0;
  list_construct(&incomplete);
  list_foreach(ts_packets_list(packets), p)
    {
      u_found = NULL;
      list_foreach(&incomplete, u)
        if ((*p)->pid == (*u)->pid)
          u_found = u;

      if ((*p)->payload_unit_start_indicator)
        {
          if (u_found)
            {
              ts_unit_compact(*u_found);
              list_erase(u_found, NULL);
            }
          unit = malloc(sizeof *unit);
          if (!unit)
            abort();
          ts_unit_construct(unit, (*p)->pid, (*p)->adaptation_field.random_access_indicator);
          if ((*p)->adaptation_field.pcr_flag)
            {
              unit->pcr_flag = 1;
              unit->pcr = (*p)->adaptation_field.pcr;
            }
          list_push_back(&incomplete, &unit, sizeof unit);
          list_push_back(&units->list, &unit, sizeof unit);
          u_found = &unit;
          count ++;
        }

      if (u_found)
        (void) ts_unit_unpack(*u_found, *p);
    }

  list_destruct(&incomplete, NULL);

  return count;
}

ssize_t ts_units_load(ts_units *units, char *path)
{
  ts_packets packets;
  ssize_t n;

  ts_packets_construct(&packets);
  n = ts_packets_load(&packets, path);
  if (n == -1)
    {
      ts_packets_destruct(&packets);
      return -1;
    }

  n = ts_units_unpack(units, &packets);
  ts_packets_destruct(&packets);
  return n;
}

ssize_t ts_units_save(ts_units *units, char *path)
{
  ts_packets packets;
  ssize_t n;

  ts_packets_construct(&packets);
  n = ts_units_pack(units, &packets);
  if (n == -1)
    {
      ts_packets_destruct(&packets);
      return -1;
    }
  n = ts_packets_save(&packets, path);
  ts_packets_destruct(&packets);
  return n;
}
