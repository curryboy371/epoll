#ifndef DRIVER_MANAGER_H
#define DRIVER_MANAGER_H

#include "define.h"

// device driver file
#define DEV_FILE_BMP "/dev/bmp180"

typedef enum {
    DRI_BMP180, 
    //DRI_BH1750,
    DRI_MAX
} DriverType;


typedef struct {
    int driver_fd;
    char driver_file_name[48];
} Driver;

typedef struct {
    Driver drivers[DRI_MAX];  
} DriverInfo;

void driver_manager_init(DriverInfo* info);

Boolean driver_manager_open(DriverInfo* info, DriverType type);

Boolean driver_manager_read(DriverInfo* info, DriverType type, char* out_buffer);


#endif // DRIVER_MANAGER_H