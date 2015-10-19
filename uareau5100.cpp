/*
Insert usual warning/disclaimer about messing with undocumented
control registers here.
*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
     
uint8_t extension_unit = 6;
uint8_t extension_guid[] = {0xb0, 0xd0, 0xbb, 0x68, 0xa4, 0x61, 0x83, 0x4b, 0x90, 0xb7, 0xa6, 0x21, 0x5f, 0x3c, 0x4f, 0x70};

uint8_t led_selector   = 7;
uint8_t led_data_on[]  = {0x81,0xf0,0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t led_data_off[] = {0x81,0xf0,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void ctrl_query(int fd, int unit, int selector, uint8_t* data, int data_size) {
    struct uvc_xu_control_query query;
    memset(&query, 0, sizeof query);
    query.unit = unit;
    query.selector = selector;
    query.query = UVC_SET_CUR;
    query.data = data;
    query.size = data_size;
    if (ioctl(fd, UVCIOC_CTRL_QUERY, &query) < 0) {
      perror("ioctl(UVCIOC_CTRL_QUERY)");
      exit(4);
    }
}

int main(int argc, const char **argv)
{
    if (argc != 3) {
      fprintf(stderr, "Usage: %s </dev/videoN> <on|off|install>\n", argv[0]);
      exit(1);
    }
    
    const char* filename = argv[1];
    const char* command = argv[2];
    
    
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
      perror("open()");
      exit(2);
    }
    
    if (strcmp(command, "on") == 0) {
      ctrl_query(fd, extension_unit, led_selector, led_data_on, sizeof(led_data_on));

    } else if (strcmp(command, "off") == 0) {
      ctrl_query(fd, extension_unit, led_selector, led_data_off, sizeof(led_data_off));
    
    } else if (strcmp(command, "install") == 0) {
    
      struct uvc_xu_control_mapping mapping;
      memset(&mapping, 0, sizeof mapping);
      mapping.id = 176;
      strcpy((char*)mapping.name, "Light");
      memcpy(mapping.entity, extension_guid, sizeof(extension_guid));
      mapping.selector = led_selector;
      mapping.size = 8; //?
      mapping.offset = 3*8; //?
      mapping.v4l2_type = V4L2_CTRL_TYPE_BOOLEAN;
      mapping.data_type = UVC_CTRL_DATA_TYPE_BOOLEAN;
      
      if (ioctl(fd, UVCIOC_CTRL_MAP, &mapping) < 0) {
        perror("ioctl(UVCIOC_CTRL_MAP)");
        exit(4);
      }
    } else {
      fprintf(stderr, "Invalid command: %s\n", command);
      exit(3);
    }
    
    return 0;
}