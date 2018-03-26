#define CMD_BUFFER_SIZE         24

#define CMD_INTF_NONE           0x00
#define CMD_INTF_USART          0x01
#define CMD_INTF_BLE            0x02

uint8_t cmd_char_process (uint8_t cmd_wptr, char cmd_char, char *cmd_buffer, uint8_t cmd_intf);

void cmd_str_exec (char *cmd_str, uint8_t cmd_intf);

void cmd_print_str (char *print_str, uint8_t cmd_intf);
void cmd_print_str_P (char *print_str, uint8_t cmd_intf);

void cmd_log (uint8_t cmd_intf);
void cmd_log_target_set (uint8_t log_target, uint8_t cmd_intf);
uint8_t cmd_log_target_get (void);