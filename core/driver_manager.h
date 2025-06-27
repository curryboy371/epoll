#ifndef DRIVER_MANAGER_H
#define DRIVER_MANAGER_H

#include "define.h"
#include <pthread.h>

// device driver file
#define DEV_FILE_BMP "/dev/bmp180"
#define DEV_FILE_LCD "/dev/lcd1602"

typedef enum {
    DRI_BMP180, 
    DRI_LCD1602,
    DRI_MAX
} DriverType;


typedef struct {
    int driver_fd;
    char driver_file_name[48];
} Driver;

typedef struct {
    Driver drivers[DRI_MAX];  
    pthread_mutex_t lock;
} DriverInfo;

void driver_manager_init(DriverInfo* info);

Boolean driver_manager_open(DriverInfo* info, DriverType type, int open_option);

Boolean driver_manager_read(DriverInfo* info, DriverType type, char* out_buffer);
Boolean driver_manager_write(DriverInfo* info, DriverType type, char* buffer);

#endif // DRIVER_MANAGER_H