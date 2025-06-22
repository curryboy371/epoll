#include "system_broadcast.h"
#include "packet.h"

#include "packet_sender.h"


#include "admin.pb-c.h"

void system_broadcast_notice(const char* message) {

    Admin__AdminMessage req = ADMIN__ADMIN_MESSAGE__INIT;
    req.message = (char*)message;

    broadcast_user_packet(CMD_ADMIN_BROADCAST, &req);
    
}