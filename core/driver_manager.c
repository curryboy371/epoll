#include "driver_manager.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
void driver_manager_init(DriverInfo* info) {

    if(!info) {
        return;
    }

    for(int i = 0; i < DRI_MAX; ++i) {

        info->drivers[i].driver_fd = -1;
        strcpy(info->drivers[i].driver_file_name, "");
    }

    pthread_mutex_init(&info->lock, NULL);

    strcpy(info->drivers[DRI_BMP180].driver_file_name, DEV_FILE_BMP);
    strcpy(info->drivers[DRI_LCD1602].driver_file_name, DEV_FILE_LCD);

}

void driver_manager_release(DriverInfo* info) {
    
    if(!info) {
        return;
    }

    pthread_mutex_lock(&info->lock); 

    for(int i = 0; i < DRI_MAX; ++i) {
        if(info->drivers[i].driver_fd != -1) {
            close(info->drivers[i].driver_fd);
            info->drivers[i].driver_fd = -1;
        }
    }

    pthread_mutex_unlock(&info->lock); 
    pthread_mutex_destroy(&info->lock);
}

Boolean driver_manager_open(DriverInfo* info, DriverType type, int open_option) {

    if(!info) {
        return FALSE;
    }

    pthread_mutex_lock(&info->lock); 
    printf("driver_manager_open %s\n", info->drivers[type].driver_file_name);
    info->drivers[type].driver_fd = open(info->drivers[type].driver_file_name, open_option);
    if (info->drivers[type].driver_fd < 0) {
        printf("Failed to open: %s %s\n", info->drivers[type].driver_file_name, strerror(errno));

        pthread_mutex_unlock(&info->lock); 
        return  FALSE;
    }

    pthread_mutex_unlock(&info->lock); 

    return TRUE;
}

Boolean driver_manager_read(DriverInfo* info, DriverType type, char* out_buffer) {

    if(!info) {
        return FALSE;
    }


    
    if(info->drivers[type].driver_fd == -1) {
        // 여기서 fd open
        if(driver_manager_open(info, type, O_RDONLY) == FALSE) {
            printf("driver open failed\n");
            return FALSE;
        }
    }


    int fd = info->drivers[type].driver_fd;
    if (fd < 0) {
        printf("Invalid driver fd: %d\n", fd);
        return FALSE;
    }

    pthread_mutex_lock(&info->lock); 

    // 다시 읽기 위해 위치 초기화
    lseek(fd, 0, SEEK_SET);

    char buf[128];
    ssize_t len = read(fd, buf, sizeof(buf) - 1);
    if (len < 0) {
        printf("Failed to device read: %s\n", strerror(errno));
        pthread_mutex_unlock(&info->lock); 
        return FALSE;
    }
    buf[len] = '\0';

    strncpy(out_buffer, buf, len);

    pthread_mutex_unlock(&info->lock); 

    return TRUE;

}

Boolean driver_manager_write(DriverInfo* info, DriverType type, char* buffer) {

    if(!info) {
        return FALSE;
    }
    
    if(info->drivers[type].driver_fd == -1) {
        // 여기서 fd open
        if(driver_manager_open(info, type, O_WRONLY) == FALSE) {
            printf("driver open failed\n");
            return FALSE;
        }
    }

    int fd = info->drivers[type].driver_fd;
    if (fd < 0) {
        printf("Invalid driver fd: %d\n", fd);
        return FALSE;
    }

    pthread_mutex_lock(&info->lock); 

    // LCD에 write
    if (write(fd, buffer, strlen(buffer)) < 0) {
        printf("Failed to device write: %s\n", strerror(errno));
        pthread_mutex_unlock(&info->lock); 
        return FALSE;
    }

    pthread_mutex_unlock(&info->lock); 

    return TRUE;

}