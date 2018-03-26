#define BLE_RX_SIZE             16

#define BLE_MSG_TYPE_CMD        0x10
#define BLE_MSG_TYPE_RESP       0x20
#define BLE_MSG_TYPE_ALERT      0x40
#define BLE_MSG_TYPE_ERROR      0x80

#define BLE_MSG_NOT_READY       0xFE
#define BLE_MSG_OVERFLOW        0xFF

#define BLE_MAX_PAYLOAD_LENGTH  16
#define BLE_MORE_DATA           7

#define BLE_CMD_TYPE_INIT       0xBEEF
#define BLE_CMD_TYPE_ATWRAP     0x0A00
#define BLE_CMD_TYPE_UARTTX     0x0A01
#define BLE_CMD_TYPE_UARTRX     0x0A02

#define BLE_CMD_FAIL            0
#define BLE_CMD_PASS            1

#define BLE_RESET_WIDTH_MS      10

extern char ble_rx_buffer[BLE_RX_SIZE];
extern uint8_t ble_rx_rptr, ble_rx_wptr;

void ble_packet_tx (uint8_t tx_msg_type, uint16_t tx_id, char *tx_payload);
void ble_packet_tx_P (uint8_t tx_msg_type, uint16_t tx_id, char *tx_payload);

void ble_packet_rx (uint8_t *rx_msg_type, uint16_t *rx_id);

uint8_t ble_send_cmd (uint16_t cmd_id, char *cmd_payload);
uint8_t ble_send_cmd_P (uint16_t cmd_id, char *cmd_payload);

void ble_reset (void);