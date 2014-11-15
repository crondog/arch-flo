
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/fb.h>

struct fb_info{
	int fd;
	void *ptr;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
};

void fb_getinfo(struct fb_info *fb_info){
	printf("fb res %dx%d virtual %dx%d, line_len %d\n",
			fb_info->var.xres, fb_info->var.yres,
			fb_info->var.xres_virtual, fb_info->var.yres_virtual,
			fb_info->fix.line_length);
	printf("dim %dmm x %dmm\n", fb_info->var.width, fb_info->var.height);
	printf("xoffset %d yoffset %d\n", fb_info->var.xoffset, fb_info->var.yoffset);
}
int fb_open(struct fb_info *fb_info){

    int fd = open("/dev/fb0", O_RDWR);

    if(fd < 0){
        printf("Failed to open fb\n");
        return 1;
    }

    memset(fb_info, 0, sizeof(struct fb_info));
    fb_info->fd = fd;
    return 0;
}


void flip_buffer(struct fb_info *fb_info, int n){
    if( ioctl(fb_info->fd, FBIOPAN_DISPLAY, &fb_info->var) < 0 ){
        perror("Failed FBIOPAN_DISPLAY");
    }
}
int main(int argc, char *argv[]){
  
    setpriority(PRIO_PROCESS, 0, -20);

    struct fb_info fb_info;
    fb_open(&fb_info);

    while(1){
        flip_buffer(&fb_info,0);
        usleep(16666);
    }
    return 0;
}
