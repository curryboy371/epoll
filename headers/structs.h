
#include "uthash.h"
typedef struct {
    char id[64];
    char password[64];
    char name[64];
    int session_fd;  // 연결된 클라이언트 fd (미연결시 -1)
    int uid;
    UT_hash_handle hh; 
} User;
