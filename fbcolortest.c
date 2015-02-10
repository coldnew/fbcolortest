/*
 * Copyright (c) 2014 Yen-Chin, Lee <coldnew.tw@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

enum COLOR_OFFSET {
        OFFSET_R = 0,
        OFFSET_G,
        OFFSET_B,
};

char color_array[][3] = {
        {  0,   0,   0},        // Black
        {255, 255, 255},        // White
        {255,   0,   0},        // Red
        {  0, 255,   0},        // Green
        {  0,   0, 255},        // Blue
        {  0, 255, 255},        // Cyan
        {160,  32, 240},        // Purple
        {255, 250,   0},        // Yellow
        {105, 105, 105},        // Dim Gray
        {190, 190, 190},        // Gray
        {211, 211, 211},        // Light Gray
};

#define COLOR_ARRAY_SIZE (sizeof(color_array) / sizeof(color_array[0]))

struct fbdevice {
        int fd;
        char *ptr;
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;
        int screensize;
};

void full_framebuffer_with_color (struct fbdevice *dev, unsigned char r, unsigned char g, unsigned char b)
{
        long int offset = 0;

        for (int x = 0; x < dev->vinfo.xres; x++) {
                for(int y = 0; y < dev->vinfo.yres; y++) {
                        offset = (x + dev->vinfo.xoffset) * (dev->vinfo.bits_per_pixel / 8) + (y + dev->vinfo.yoffset) * dev->finfo.line_length;
                        if (32 == dev->vinfo.bits_per_pixel) {
                                *((unsigned int *) (dev->ptr + offset + OFFSET_R)) = r;
                                *((unsigned int *) (dev->ptr + offset + OFFSET_G)) = g;
                                *((unsigned int *) (dev->ptr + offset + OFFSET_B)) = b;
                        }
                        else {
                                // assume 16bpp
                                *((unsigned int *) (dev->ptr + offset)) = (r << 11) | (g << 5) | b;
                        }
                }
        }
}

void clear_framebuffer (struct fbdevice *dev)
{
        full_framebuffer_with_color(dev, 0, 0, 0);
}

int fbcolortest_device_open (struct fbdevice *dev)
{
        int ret = -1;

        dev->fd = open("/dev/fb0", O_RDWR);
        if (dev->fd < 0) {
                dev->fd = open("/dev/graphics/fb0", O_RDWR);
                if (dev->fd < 0) {
                        fprintf(stderr, "open framebuffer failed, error: %s\n", strerror(errno));
                        return dev->fd;
                }
        }

        // Get fixed screen information
        ret = ioctl(dev->fd, FBIOGET_FSCREENINFO, &dev->finfo);
        if (ret < 0) {
                fprintf(stderr, "Error reading dev fixed information, error: %s\n", strerror(errno));
                return ret;
        }

        // Get variable screen information
        ret = ioctl(dev->fd, FBIOGET_VSCREENINFO, &dev->vinfo);
        if (ret < 0) {
                fprintf(stderr, "Error reading dev variable information, error: %s\n", strerror(errno));
                return ret;
        }

        fprintf(stdout, "Screen information as : %d x %d, %dbpp\n", dev->vinfo.xres, dev->vinfo.yres, dev->vinfo.bits_per_pixel);

        // Calculate the size of the screen in bytes
        dev->screensize = dev->vinfo.xres_virtual * dev->vinfo.yres_virtual * (dev->vinfo.bits_per_pixel / 8);

        // mapping device to memory
        dev->ptr = (char *) mmap(0, dev->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, 0);
        if (MAP_FAILED == (int) dev->ptr) {
                fprintf(stderr, "Failed to map framebuffer device to memory, error: %s\n", strerror(errno));
                return (int) dev->ptr;
        }

        fprintf(stdout, "The framebuffer device was mapped to memory successfully.\n");

        return 0;
}

void fbcolortest_device_close (struct fbdevice *dev)
{
        // clear framebuffer
        clear_framebuffer(dev);

        // release resource
        munmap (dev->ptr, dev->screensize);
        close(dev->fd);
}

int main (int argc, char *argv[])
{
        int ret = -1;
        struct fbdevice dev;

        ret = fbcolortest_device_open(&dev);
        if (ret < 0) {
                close(dev.fd);
                exit(EXIT_FAILURE);
        }

        while (1) {
                for (int i = 0; i < COLOR_ARRAY_SIZE; i++) {
                        full_framebuffer_with_color(&dev, color_array[i][0], color_array[i][1], color_array[i][2]);
                        sleep(1);
                }
        }

        fbcolortest_device_close(&dev);
        return 0;
}
