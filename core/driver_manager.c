#include "driver_manager.h"

#include <fcntl.h>
#include <unistd.h>

void driver_manager_init(DriverInfo* info) {

    if(!info) {
        return;
    }

    for(int i = 0; i < DRI_MAX; ++i) {

        info->drivers[i].driver_fd = -1;
        strcpy(info->drivers[i].driver_file_name, "");
    }

    strcpy(info->drivers[DRI_BMP180].driver_file_name, DEV_FILE_BMP);
    //strcpy(info->drivers[DRI_BMP180].driver_file_name, DEV_FILE_BMP);

}


Boolean driver_manager_open(DriverInfo* info, DriverType type) {

    if(!info) {
        return FALSE;
    }

    info->drivers[type].driver_fd = open(info->drivers[type].driver_file_name, O_RDONLY);
    if (info->drivers[type].driver_fd < 0) {
        perror("Failed to open");
        return  FALSE;
    }
}

Boolean driver_manager_read(DriverInfo* info, DriverType type, char* out_buffer) {

    if(!info) {
        return FALSE;
    }
    
    int fd = info->drivers[type].driver_fd;
    if(fd == -1) {
        // 여기서 fd open
        if(driver_manager_open(info, type)) {
            printf("driver open failed\n");
            return FALSE;
        }
    }

    char buf[128];
    ssize_t len = read(fd, buf, sizeof(buf) - 1);
    if (len < 0) {
        printf("driver read failed\n");
        perror("Failed to read from device");
        return FALSE;
    }
    buf[len] = '\0';

    strncpy(out_buffer, buf, len);
}
