#include <string.h>

typedef struct{
    char data[128];
    char length;
} rs485_packet_buffer_t;

rs485_packet_buffer_t tx_buffer;
rs485_packet_buffer_t rx_buffer;

size_t rs485_rx_len(rs485_t *dev)
{
    return (size_t) dev->rx_buffer.length;
}

void rs485_rx_read(rs485_t *dev, uint8_t *data, size_t len,
                   size_t offset)
{
    memcpy(data, (dev->rx_buffer) + offset, len);
}
