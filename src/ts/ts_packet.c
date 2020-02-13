#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <dynamic.h>

#include "bytestream.h"
#include "ts_ebp.h"
#include "ts_adaptation_field.h"
#include "ts_packet.h"

void ts_packet_construct(ts_packet *packet)
{
  *packet = (ts_packet) {0};
  ts_adaptation_field_construct(&packet->adaptation_field);
}

void ts_packet_destruct(ts_packet *packet)
{
  ts_adaptation_field_destruct(&packet->adaptation_field);
  free(packet->payload_data);
}

size_t ts_packet_size(ts_packet *packet)
{
  return 4 +
    (packet->adaptation_field_control & 0x02 ? ts_adaptation_field_size(&packet->adaptation_field) : 0) +
    packet->payload_size;
}

ssize_t ts_packet_pack(ts_packet *packet, bytestream *stream)
{
  ssize_t n;

  if (packet->payload_size)
    packet->adaptation_field_control |= 0x01;
  else
    packet->adaptation_field_control &= ~0x01;

  if (ts_packet_size(packet) < 188)
    packet->adaptation_field_control |= 0x02;

  bytestream_write32(stream,
                 bytestream_write_bits(0x47, 32, 0, 8) |
                 bytestream_write_bits(packet->transport_error_indicator, 32, 8, 1) |
                 bytestream_write_bits(packet->payload_unit_start_indicator, 32, 9, 1) |
                 bytestream_write_bits(packet->transport_priority, 32, 10, 1) |
                 bytestream_write_bits(packet->pid, 32, 11, 13) |
                 bytestream_write_bits(packet->transport_scrambling_control, 32, 24, 2) |
                 bytestream_write_bits(packet->adaptation_field_control, 32, 26, 2) |
                 bytestream_write_bits(packet->continuity_counter, 32, 28, 4));

  if (packet->adaptation_field_control & 0x02)
    {
      n = ts_adaptation_field_pack(&packet->adaptation_field, stream, 188 - 4 - packet->payload_size);
      if (n == -1)
        return -1;
    }

  if (packet->adaptation_field_control & 0x01)
    bytestream_write(stream, packet->payload_data, packet->payload_size);

  return bytestream_valid(stream) ? 1 : -1;
}

ssize_t ts_packet_unpack(ts_packet *packet, bytestream *stream)
{
  size_t size;
  ssize_t n;
  uint32_t v;

  size = bytestream_size(stream);
  if (size < 188)
    return 0;

  v = bytestream_read32(stream);
  packet->transport_error_indicator = bytestream_read_bits(v, 32, 8, 1);
  packet->payload_unit_start_indicator = bytestream_read_bits(v, 32, 9, 1);
  packet->transport_priority = bytestream_read_bits(v, 32, 10, 1);
  packet->pid = bytestream_read_bits(v, 32, 11, 13);
  packet->transport_scrambling_control = bytestream_read_bits(v, 32, 24, 2);
  packet->adaptation_field_control = bytestream_read_bits(v, 32, 26, 2);
  packet->continuity_counter = bytestream_read_bits(v, 32, 28, 4);

  if (packet->adaptation_field_control & 0x02)
    {
      n = ts_adaptation_field_unpack(&packet->adaptation_field, stream);
      if (n <= 0)
        return n;
    }

  if (packet->adaptation_field_control & 0x01)
    {
      packet->payload_size = 188 - (size - bytestream_size(stream));
      packet->payload_data = malloc(packet->payload_size);
      if (!packet->payload_data)
        abort();
      bytestream_read(stream, packet->payload_data, packet->payload_size);
    }

  return bytestream_valid(stream) ? 1 : -1;
}

void ts_packet_debug(ts_packet *packet, FILE *f)
{
  uint32_t s1, s2;
  size_t i;
  bytestream stream;
  buffer buffer;

  s1 = 0;
  for (i = 0; i < packet->payload_size; i ++)
    s1 += ((uint8_t *) packet->payload_data)[i];

  buffer_construct(&buffer);
  bytestream_construct_buffer(&stream, &buffer);
  ts_packet_pack(packet, &stream);

  s2 = 0;
  for (i = 0; i < bytestream_size(&stream); i ++)
    s2 += ((uint8_t *) bytestream_data(&stream))[i];

  bytestream_destruct(&stream);
  buffer_destruct(&buffer);

  (void) fprintf(f, "[pid %u, tei %u, pusi %u, prio %u, tsc %u, afc %u, cc %u, size %lu, payload sum %08x, packet sum %08x]\n",
                 packet->pid, packet->transport_error_indicator, packet->payload_unit_start_indicator,
                 packet->transport_priority, packet->transport_scrambling_control, packet->adaptation_field_control,
                 packet->continuity_counter, packet->payload_size, s1, s2);
}
