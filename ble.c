#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

#include "ble.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"

void ble_packet_tx (uint8_t tx_msg_type, uint16_t tx_id, char *tx_payload)
{

    uint8_t tx_length, tx_offset, i;

    tx_length = strlen (tx_payload);
    tx_offset = 0;

    do {

        // assert chip select
        PORTB &= ~(1 << BT_CSn);
        _delay_us (BLE_START_DELAY_US);

        // send header and check for not-ready response
        while (spi_xfer (tx_msg_type) == BLE_MSG_NOT_READY)
        {

            // de-assert chip select and wait
            PORTB |= (1 << BT_CSn);
            _delay_us (BLE_RETRY_DELAY_US);

            // re-assert chip select
            PORTB &= ~(1 << BT_CSn);
            _delay_us (BLE_START_DELAY_US);

        } // while (not ready)

        // send command type
        spi_xfer ((uint8_t)tx_id);
        spi_xfer ((uint8_t)(tx_id >> 8));

        // manage payloads that exceed maximum length
        if (tx_length > BLE_MAX_PAYLOAD_LENGTH)
        {

            // send maximum length with "more data" flag set
            spi_xfer (BLE_MAX_PAYLOAD_LENGTH | (1 << BLE_MORE_DATA));

            // send payload
            for (i = 0; i < BLE_MAX_PAYLOAD_LENGTH; i++) spi_xfer ((uint8_t)tx_payload[tx_offset + i]);

            // calculate remaining length
            tx_length -= BLE_MAX_PAYLOAD_LENGTH;
            tx_offset += BLE_MAX_PAYLOAD_LENGTH;

        } else {

            // send remaining length
            spi_xfer (tx_length);

            // send payload
            for (i = 0; i < tx_length; i++) spi_xfer ((uint8_t)tx_payload[tx_offset + i]);

            // calculate remaining length
            tx_length = 0;

        } // if (tx_length)

        // de-assert chip select
        PORTB |= (1 << BT_CSn);

    } while (tx_length);

}

void ble_packet_tx_P (uint8_t tx_msg_type, uint16_t tx_id, char *tx_payload)
{

    uint8_t tx_length, tx_offset, i;

    tx_length = strlen_P (tx_payload);
    tx_offset = 0;

    do {

        // assert chip select
        PORTB &= ~(1 << BT_CSn);
        _delay_us (BLE_START_DELAY_US);

        // send header and check for not-ready response
        while (spi_xfer (tx_msg_type) == BLE_MSG_NOT_READY)
        {

            // de-assert chip select and wait
            PORTB |= (1 << BT_CSn);
            _delay_us (BLE_RETRY_DELAY_US);

            // re-assert chip select
            PORTB &= ~(1 << BT_CSn);
            _delay_us (BLE_START_DELAY_US);

        } // while (not ready)

        // send command type
        spi_xfer ((uint8_t)tx_id);
        spi_xfer ((uint8_t)(tx_id >> 8));

        // manage payloads that exceed maximum length
        if (tx_length > BLE_MAX_PAYLOAD_LENGTH)
        {

            // send maximum length with "more data" flag set
            spi_xfer (BLE_MAX_PAYLOAD_LENGTH | (1 << BLE_MORE_DATA));

            // send payload
            for (i = 0; i < BLE_MAX_PAYLOAD_LENGTH; i++) spi_xfer ((uint8_t)pgm_read_byte (&(tx_payload[tx_offset + i])));

            // calculate remaining length
            tx_length -= BLE_MAX_PAYLOAD_LENGTH;
            tx_offset += BLE_MAX_PAYLOAD_LENGTH;

        } else {

            // send remaining length
            spi_xfer (tx_length);

            // send payload
            for (i = 0; i < tx_length; i++) spi_xfer ((uint8_t)pgm_read_byte (&(tx_payload[tx_offset + i])));

            // calculate remaining length
            tx_length = 0;

        } // if (tx_length)

        // de-assert chip select
        PORTB |= (1 << BT_CSn);

    } while (tx_length);

}

void ble_packet_rx (uint8_t *rx_msg_type, uint16_t *rx_id)
{

    uint8_t rx_length, i;
    char rx_data;

    do {

        // assert chip select
        PORTB &= ~(1 << BT_CSn);
        _delay_us (BLE_START_DELAY_US);

        // capture header (first try)
        *rx_msg_type = spi_xfer (0xFF);

        // check for not-ready or overflow responses
        while ((*rx_msg_type == BLE_MSG_NOT_READY) || (*rx_msg_type == BLE_MSG_OVERFLOW))
        {

            // de-assert chip select and wait
            PORTB |= (1 << BT_CSn);
            _delay_us (BLE_RETRY_DELAY_US);

            // re-assert chip select
            PORTB &= ~(1 << BT_CSn);
            _delay_us (BLE_START_DELAY_US);

            // retry header
            *rx_msg_type = spi_xfer (0xFF);

        } // while (not ready)

        // capture response type
        *rx_id = (uint16_t)spi_xfer (0xFF);
        *rx_id += ((uint16_t)spi_xfer (0xFF) << 8);

        // capture length
        rx_length = spi_xfer (0xFF);

        // capture payload
        for (i = 0; i < (rx_length & ~(1 << BLE_MORE_DATA)); i++)
        {

            // capture character
            rx_data = (char)spi_xfer (0xFF);

            // manage character based on command ID
            switch (*rx_id)
            {

                case BLE_CMD_TYPE_ATWRAP:

                    // echo character to terminal
                    usart_print_char (rx_data);

                    break;

                case BLE_CMD_TYPE_UARTRX:

                    // queue character and increment write pointer
                    ble_rx_buffer[ble_rx_wptr++] = rx_data;

                    // wrap write pointer
                    ble_rx_wptr %= BLE_RX_SIZE;

                    break;

            } // switch (command ID)

        } // for (i)

        // de-assert chip select
        PORTB |= (1 << BT_CSn);

    } while (rx_length & (1 << BLE_MORE_DATA));

}

uint8_t ble_send_cmd (uint16_t cmd_id, char *cmd_payload)
{

    uint8_t resp_msg_type;
    uint16_t resp_id;

    // send command packet
    ble_packet_tx (BLE_MSG_TYPE_CMD, cmd_id, cmd_payload);

    // wait for interrupt
    while (PIND & (1 << BT_IRQn));

    // retrieve response packet
    ble_packet_rx (&resp_msg_type, &resp_id);

    // check message type and compare command IDs
    if ((resp_msg_type == BLE_MSG_TYPE_RESP) && (resp_id == cmd_id))
    {

        return (BLE_CMD_PASS);

    } else {

        return (BLE_CMD_FAIL);

    } // if (...)

}

uint8_t ble_send_cmd_P (uint16_t cmd_id, char *cmd_payload)
{

    uint8_t resp_msg_type;
    uint16_t resp_id;

    // send command packet
    ble_packet_tx_P (BLE_MSG_TYPE_CMD, cmd_id, cmd_payload);

    // wait for interrupt
    while (PIND & (1 << BT_IRQn));

    // retrieve response packet
    ble_packet_rx (&resp_msg_type, &resp_id);

    // check message type and compare command IDs
    if ((resp_msg_type == BLE_MSG_TYPE_RESP) && (resp_id == cmd_id))
    {

        return (BLE_CMD_PASS);

    } else {

        return (BLE_CMD_FAIL);

    } // if (...)

}

void ble_reset (void)
{

    // assert reset and wait
    PORTB &= ~(1 << BT_RSTn);
    _delay_ms (BLE_RESET_WIDTH_MS);

    // release reset
    PORTB |= (1 << BT_RSTn);

}