#pragma once

#include <cutils.h>
#include <virtio.h>
#include <conio.h>
struct winsize
{
    unsigned short ws_row;    /* rows, in characters */
    unsigned short ws_col;    /* columns, in characters */
    unsigned short ws_xpixel; /* horizontal size, pixels */
    unsigned short ws_ypixel; /* vertical size, pixels */
};

typedef struct
{
    // int stdin_fd;
    int console_esc_state;
    BOOL resize_pending;
} STDIODevice;

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>

#ifdef _WIN32
// Some old MinGW/CYGWIN distributions don't define this:
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static HANDLE stdoutHandle;
static DWORD outModeInit;

void setupConsole(void)
{
    DWORD outMode = 0;
    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (stdoutHandle == INVALID_HANDLE_VALUE)
    {
        exit(GetLastError());
    }

    if (!GetConsoleMode(stdoutHandle, &outMode))
    {
        exit(GetLastError());
    }

    outModeInit = outMode;

    // Enable ANSI escape codes
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(stdoutHandle, outMode))
    {
        exit(GetLastError());
    }
}

void restoreConsole(void)
{
    // Reset colors
    printf("\x1b[0m");

    // Reset console mode
    if (!SetConsoleMode(stdoutHandle, outModeInit))
    {
        exit(GetLastError());
    }
}
#else
void setupConsole(void)
{
}

void restoreConsole(void)
{
    // Reset colors
    printf("\x1b[0m");
}
#endif

static void simple_console_get_size(STDIODevice *s, int *pw, int *ph)
{
    struct winsize ws;
    int width, height;
    /* default values */
    width = 80;
    height = 25;
    *pw = width;
    *ph = height;
}

static int simple_console_read(void *opaque, uint8_t *buf, int len)
{
    int i = 0;
    while (kbhit() && i < len)
    {
        buf[i] = getch();
        i++;
    }

    return i;
}

static void simple_console_write(void *opaque, const uint8_t *buf, int len)
{
    (void)(opaque);
    fwrite(buf, 1, len, stdout);
    fflush(stdout);
}

CharacterDevice *simple_console_init(BOOL allow_ctrlc)
{
    (void)(allow_ctrlc);
    setupConsole();
    CharacterDevice *dev;
    STDIODevice *s;

    dev = mallocz(sizeof(*dev));
    s = mallocz(sizeof(*s));
    s->resize_pending = TRUE;

    dev->opaque = s;
    dev->write_data = simple_console_write;
    dev->read_data = simple_console_read;
    return dev;
}