#pragma once

#include <cutils.h>
#include <virtio.h>
#include <driver/uart.h>

typedef struct
{
    int console_esc_state;
    BOOL resize_pending;
} STDIODevice;

#include <stdio.h>

static void uart_console_get_size(STDIODevice *s, int *pw, int *ph)
{
    struct winsize ws;
    int width, height;
    /* default values */
    width = 80;
    height = 25;
    *pw = width;
    *ph = height;
}

static int uart_console_read(void *opaque, uint8_t *buf, int len)
{
    return uart_read_bytes(CONFIG_ESP_CONSOLE_UART_NUM, buf, len,0);
}

static void uart_console_write(void *opaque, const uint8_t *buf, int len)
{
    uart_write_bytes(CONFIG_ESP_CONSOLE_UART_NUM, buf, len);
}

void setupConsole(void)
{
    printf("Setting up UART Console\r\n");
    
    ESP_ERROR_CHECK(uart_driver_install((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM,
                                        256, 0, 0, NULL, 0));

    const char *tester = "UART Console Active\r\n";
    uart_console_write(NULL, (uint8_t *)tester, strlen(tester));
}

void restoreConsole(void)
{
}

CharacterDevice *uart_console_init(BOOL allow_ctrlc)
{
    setupConsole();
    CharacterDevice *dev;
    STDIODevice *s;

    dev = mallocz(sizeof(*dev));
    s = mallocz(sizeof(*s));
    s->resize_pending = TRUE;

    dev->opaque = s;
    dev->write_data = uart_console_write;
    dev->read_data = uart_console_read;
    return dev;
}
