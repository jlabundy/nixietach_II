#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "usart.h"
#include "calc.h"
#include "display.h"
#include "gpio.h"
#include "ble.h"

char CMD_MSG_CURSOR[] PROGMEM = "> ";
char CMD_MSG_CRLF[] PROGMEM = "\r\n";
char CMD_MSG_OK[] PROGMEM = "OK\r\n";
char CMD_MSG_INVALID[] PROGMEM = "Invalid entry\r\n";

char CMD_MSG_VER[] PROGMEM = "NIXIETACH II Command-Line Interface\r\n" __DATE__ " " __TIME__ "\r\n";

char CMD_MSG_8CYL[] PROGMEM = "8 cylinders\r\n";
char CMD_MSG_6CYL[] PROGMEM = "6 cylinders\r\n";
char CMD_MSG_4CYL[] PROGMEM = "4 cylinders\r\n";

char CMD_MSG_LOG_SEG1[] PROGMEM = "RPM: ";
char CMD_MSG_LOG_SEG2[] PROGMEM = ", dwell: ";
char CMD_MSG_LOG_SEG3[] PROGMEM = " deg.\r\n";
char CMD_MSG_LOG_SEGx[] PROGMEM = "---";
char CMD_MSG_LOG_START[] PROGMEM = "Logging started\r\n";
char CMD_MSG_LOG_STOP[] PROGMEM = "Logging stopped\r\n";

char CMD_MSG_DISP_ON[] PROGMEM = "Display is on\r\n";
char CMD_MSG_DISP_OFF[] PROGMEM = "Display is off\r\n";

char CMD_MSG_BLE_ERROR[] PROGMEM = "Unexpected response from BLE module\r\n";

char CMD_TOK_VER[] PROGMEM = "VER";

char CMD_TOK_CYL[] PROGMEM = "CYL";

char CMD_TOK_RPM[] PROGMEM = "RPM";
char CMD_TOK_START[] PROGMEM = "START";
char CMD_TOK_STOP[] PROGMEM = "STOP";

char CMD_TOK_DISP[] PROGMEM = "DISP";
char CMD_TOK_ON[] PROGMEM = "ON";
char CMD_TOK_OFF[] PROGMEM = "OFF";
char CMD_TOK_RESET[] PROGMEM = "RESET";

char CMD_TOK_BLE[] PROGMEM = "BLE";
char CMD_TOK_INIT[] PROGMEM = "INIT";

static uint8_t cmd_log_target;

uint8_t cmd_char_process (uint8_t cmd_wptr, char cmd_char, char *cmd_buffer, uint8_t cmd_intf)
{

    // interpret character
    switch (cmd_char)
    {

        case 0x0D:          // carriage return
        case 0x0A:          // line feed

            // append null terminator
            cmd_buffer[cmd_wptr] = '\0';

            // execute buffer contents
            cmd_str_exec (cmd_buffer, cmd_intf);

            // reset pointer
            return (0);

        case 0x7F:          // backspace

            // protect against under-run
            if (cmd_wptr > 0)
            {

                // walk cursor back (terminal only)
                if (cmd_intf == CMD_INTF_USART) usart_print_char (0x7F);

                // decrement pointer
                return (cmd_wptr - 1);

            } else {

                // leave pointer unchanged
                return (cmd_wptr);

            } // if (pointer)

        default:            // all other characters

            // protect against over-run
            if (cmd_wptr < CMD_BUFFER_SIZE)
            {

                // store character
                cmd_buffer[cmd_wptr] = cmd_char;

                // echo character (terminal only)
                if (cmd_intf == CMD_INTF_USART) usart_print_char (cmd_char);

                // increment pointer
                return (cmd_wptr + 1);

            } else {

                // leave pointer unchanged
                return (cmd_wptr);

            } // if (pointer)

    } // switch (character)

}

void cmd_str_exec (char *cmd_str, uint8_t cmd_intf)
{

    char *cmd_token;
    cmd_token = strtok (cmd_str, " ");

    // force carriage return + line feed (terminal only)
    if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CRLF);

    // interpret command
    if (!cmd_token)
    {

        // print cursor (terminal only)
        if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CURSOR);

    } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_VER))) {

        // print version info
        cmd_print_str_P (CMD_MSG_VER, cmd_intf);

        // print cursor (terminal only)
        if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CURSOR);

    } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_CYL))) {

        // print state of cylinder-select switch
        switch (CSW_STATE)
        {

            case CSW_STATE_8CYL:

                cmd_print_str_P (CMD_MSG_8CYL, cmd_intf);
                break;

            case CSW_STATE_6CYL:

                cmd_print_str_P (CMD_MSG_6CYL, cmd_intf);
                break;

            case CSW_STATE_4CYL:

                cmd_print_str_P (CMD_MSG_4CYL, cmd_intf);
                break;

            default:

                cmd_print_str_P (CMD_MSG_INVALID, cmd_intf);

        } // switch (cylinder select)

        // print cursor (terminal only)
        if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CURSOR);

    } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_RPM))) {

        // retrieve next argument
        cmd_token = strtok (NULL, " ");

        // interpret argument
        if (!cmd_token)
        {

            // print RPM and dwell once
            cmd_log (cmd_intf);

            // print cursor (terminal only)
            if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CURSOR);

        } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_START))) {

            // set selected interface as logging target interface
            cmd_log_target_set (cmd_intf, cmd_intf);

        } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_STOP))) {

            // reset logging target interface
            cmd_log_target_set (CMD_INTF_NONE, cmd_intf);

        } else {

            // print error message
            cmd_print_str_P (CMD_MSG_INVALID, cmd_intf);

            // print cursor (terminal only)
            if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CURSOR);

        } // if (argument)

    } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_DISP))) {

        // retrieve next argument
        cmd_token = strtok (NULL, " ");

        // interpret argument
        if (!cmd_token)
        {

            // print display state
            if (DISPLAY_STATE == DISPLAY_STATE_ON) cmd_print_str_P (CMD_MSG_DISP_ON, cmd_intf); else cmd_print_str_P (CMD_MSG_DISP_OFF, cmd_intf);

        } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_ON))) {

            // enable display
            display_force_on ();

            // print confirmation message
            cmd_print_str_P (CMD_MSG_DISP_ON, cmd_intf);

        } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_OFF))) {

            // disable display
            display_force_off ();

            // print confirmation message
            cmd_print_str_P (CMD_MSG_DISP_OFF, cmd_intf);

        } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_RESET))) {

            // reset display mode
            display_mode_set (DISPLAY_MODE_NORM);

            // print confirmation message
            cmd_print_str_P (CMD_MSG_OK, cmd_intf);

        } else {

            // change display mode
            display_mode_set (DISPLAY_MODE_DEMO);

            // display argument
            display_print_num (atoi (cmd_token));

            // print confirmation message
            cmd_print_str_P (CMD_MSG_OK, cmd_intf);

        } // if (argument)

        // print cursor (terminal only)
        if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CURSOR);

    } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_BLE))) {

        // prevent BLE commands from being sent over BLE
        if (cmd_intf == CMD_INTF_BLE)
        {

            // display error message
            ble_send_cmd_P (BLE_CMD_TYPE_UARTTX, CMD_MSG_INVALID);

        } else {

            // retrieve next argument
            cmd_token = strtok (NULL, " ");

            // interpret argument
            if (!(strcmp_P (strupr (cmd_token), CMD_TOK_RESET)))
            {

                // toggle reset
                ble_reset ();

                // print confirmation message
                usart_print_str_P (CMD_MSG_OK);

            } else if (!(strcmp_P (strupr (cmd_token), CMD_TOK_INIT))) {

                // send initialization command
                if (ble_send_cmd (BLE_CMD_TYPE_INIT, "") == BLE_CMD_PASS)
                {

                    // print confirmation message
                    usart_print_str_P (CMD_MSG_OK);

                } else {

                    // print error message
                    usart_print_str_P (CMD_MSG_BLE_ERROR);

                } // if (pass)

            } else {

                // send AT command
                ble_send_cmd (BLE_CMD_TYPE_ATWRAP, cmd_token);

            } // if (argument)

            // print cursor
            usart_print_str_P (CMD_MSG_CURSOR);

        } // if (interface)

    } else {

        // print error message
        cmd_print_str_P (CMD_MSG_INVALID, cmd_intf);

        // print cursor (terminal only)
        if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CURSOR);

    } // if (command)

}

void cmd_print_str (char *print_str, uint8_t cmd_intf)
{

    // print string to selected interface
    switch (cmd_intf)
    {

        case CMD_INTF_USART:

            usart_print_str (print_str);
            break;

        case CMD_INTF_BLE:

            ble_send_cmd (BLE_CMD_TYPE_UARTTX, print_str);
            break;

    } // switch (interface)

}

void cmd_print_str_P (char *print_str, uint8_t cmd_intf)
{

    // print string (from program memory) to selected interface
    switch (cmd_intf)
    {

        case CMD_INTF_USART:

            usart_print_str_P (print_str);
            break;

        case CMD_INTF_BLE:

            ble_send_cmd_P (BLE_CMD_TYPE_UARTTX, print_str);
            break;

    } // switch (interface)

}

void cmd_log (uint8_t cmd_intf)
{

    char temp_str[8];

    // print leading segment
    cmd_print_str_P (CMD_MSG_LOG_SEG1, cmd_intf);

    // print RPM (unless engine is stalled)
    if (calc_rpm_get ()) utoa (calc_rpm_get (), temp_str, 10); else strcpy_P (temp_str, CMD_MSG_LOG_SEGx);
    cmd_print_str (temp_str, cmd_intf);

    // print middle segment
    cmd_print_str_P (CMD_MSG_LOG_SEG2, cmd_intf);

    // print dwell (unless engine is stalled)
    if (calc_dwell_get ()) utoa (calc_dwell_get (), temp_str, 10); else strcpy_P (temp_str, CMD_MSG_LOG_SEGx);
    cmd_print_str (temp_str, cmd_intf);

    // print trailing segment
    cmd_print_str_P (CMD_MSG_LOG_SEG3, cmd_intf);

}

void cmd_log_target_set (uint8_t log_target, uint8_t cmd_intf)
{

    // set data logging target interface
    cmd_log_target = log_target;

    // print appropriate confirmation message
    switch (cmd_log_target)
    {

        case CMD_INTF_USART:
        case CMD_INTF_BLE:

            cmd_print_str_P (CMD_MSG_LOG_START, cmd_intf);
            break;

        case CMD_INTF_NONE:

            cmd_print_str_P (CMD_MSG_LOG_STOP, cmd_intf);

            // print cursor (terminal only)
            if (cmd_intf == CMD_INTF_USART) usart_print_str_P (CMD_MSG_CURSOR);

            break;

    } // switch (interface)

}

uint8_t cmd_log_target_get (void)
{

    // get data logging target interface
    return (cmd_log_target);

}