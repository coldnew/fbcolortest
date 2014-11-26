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

void full_framebuffer (struct fbdevice *dev, unsigned char r, unsigned char g, unsigned char b)
{
        long int offset = 0;

        for (int x = 0; x < dev->vinfo.xres; x++) {
                for(int y = 0; y < dev->vinfo.yres - 1; y++) {
                        offset = (x + dev->vinfo.xoffset) * (dev->vinfo.bits_per_pixel / 8) + (y + dev->vinfo.yoffset) * dev->finfo.line_length;
                        *((unsigned int *) (dev->ptr + offset + OFFSET_R)) = r;
                        *((unsigned int *) (dev->ptr + offset + OFFSET_G)) = g;
                        *((unsigned int *) (dev->ptr + offset + OFFSET_B)) = b;
                }
        }
}

void clear_framebuffer (struct fbdevice *dev)
{
        full_framebuffer(dev, 0, 0, 0);
}

int main (int argc, char *argv[])
{
        int ret = -1;
        struct fbdevice fbdev;

        fbdev.fd = open("/dev/fb0", O_RDWR);
        if (fbdev.fd < 0) {
                fbdev.fd = open("/dev/graphics/fb0", O_RDWR);
                if (fbdev.fd < 0) {
                        fprintf(stderr, "open framebuffer failed, error: %s\n", strerror(errno));
                        exit (1);
                }
        }

        // Get fixed screen information
        ret = ioctl(fbdev.fd, FBIOGET_FSCREENINFO, &fbdev.finfo);
        if (ret < 0) {
                fprintf(stderr, "Error reading fbdev fixed information, error: %s\n", strerror(errno));
                exit(2);
        }

        // Get variable screen information
        ret = ioctl(fbdev.fd, FBIOGET_VSCREENINFO, &fbdev.vinfo);
        if (ret < 0) {
                fprintf(stderr, "Error reading fbdev variable information, error: %s\n", strerror(errno));
                exit(3);
        }

        fprintf(stdout, "Screen information as : %d x %d, %dbpp\n", fbdev.vinfo.xres, fbdev.vinfo.yres, fbdev.vinfo.bits_per_pixel);

        // Calculate the size of the screen in bytes
        fbdev.screensize = fbdev.vinfo.xres_virtual * fbdev.vinfo.yres_virtual * (fbdev.vinfo.bits_per_pixel / 8);

        // mapping device to memory
        fbdev.ptr = (char *) mmap(0, fbdev.screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev.fd, 0);
        if (-1 == (int) fbdev.ptr) {
                fprintf(stderr, "Failed to map framebuffer device to memory, error: %s\n", strerror(errno));
                exit(5);
        }

        fprintf(stdout, "The framebuffer device was mapped to memory successfully.\n");

        for (int i = 0; i < COLOR_ARRAY_SIZE; i++) {
                full_framebuffer(&fbdev, color_array[i][0], color_array[i][1], color_array[i][2]);
                sleep(1);
        }

        // clear framebuffer
        clear_framebuffer(&fbdev);

        // release resource
        munmap (fbdev.ptr, fbdev.screensize);
        close(fbdev.fd);

        return 0;
}
