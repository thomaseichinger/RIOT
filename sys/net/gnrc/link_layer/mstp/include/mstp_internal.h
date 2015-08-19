
#define MSTP_FRAME_TYPE_TOKEN                       (0x00)
#define MSTP_FRAME_TYPE_POLL_FOR_MASTER             (0x01)
#define MSTP_FRAME_TYPE_REPLY_POLL_FOR_MASTER       (0x02)
#define MSTP_FRAME_TYPE_TEST_REQUEST                (0x03)
#define MSTP_FRAME_TYPE_TEST_RESPONSE               (0x04)
#define MSTP_FRAME_TYPE_DATA_EXPECTING_REPLY        (0x05)
#define MSTP_FRAME_TYPE_DATA_NOT_EXPECTING_REPLY    (0x06)
#define MSTP_FRAME_TYPE_REPLY_POSTPONED             (0x07)

typedef struct
{
    uint8_t valid;
    uint8_t type;
    uint8_t dst_addr;
    uint8_t src_addr;
    uint16_t length;
    uint8_t header_crc;
    uint8_t data[1500];
    uint16_t data_crc;
} mstp_frame_t;

typedef struct
{
    uart_t uart;
    kernel_pid_t mac_pid;
    uint8_t state;
    mstp_frame_t frame;
    uint8_t addr;

} mstp_slave_t;
