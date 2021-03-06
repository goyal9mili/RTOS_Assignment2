#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include "message.h"

// SPECIFICATIONS FOR PA
static const pa_sample_spec ss = {
    .format = PA_SAMPLE_S16LE,
    .rate = 44100,
    .channels = 2
};



/* A simple routine calling UNIX write() in a loop */
static ssize_t loop_write(int fd, const void* data, size_t size) {
    ssize_t ret = 0;
    while (size > 0) {
        ssize_t r;

        if ((r = write(fd, data, size)) < 0)
            return r;

        if (r == 0)
            break;

        ret += r;
        data = (const struct Message *)data + r;
        size -= (size_t) r;
    }
    return ret;
}

/* A simple routine calling UNIX write() in a loop */
static ssize_t loop_read(int fd, void* data, size_t size) {
    ssize_t ret = 0;
    while (size > 0) {
        ssize_t r;

        if ((r = read(fd, data, size)) < 0)
            return r;

        if (r == 0)
            break;
        ret += r;
        data = (struct Message*)data + r;
        size -= (size_t) r;
    }
    return ret;
}


