#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include "define.h"
#include "user_manager.h"
#include <netdb.h>
#include <mongoc/mongoc.h>

typedef struct {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *user_collection;


} DBInfo;

Boolean db_init(DBInfo *manager, const char *uri, const char *dbname);
void db_release(DBInfo *manager);
void db_cleanup(DBInfo *manager);

Boolean db_find_user_by_pw(DBInfo *manager, const char *user_id, const char *password, User* out_user);
Boolean db_find_all_users(DBInfo *manager, User* out_users, size_t* out_count);
Boolean db_insert_user(DBInfo *manager, const char *user_id, const char *password, const char *nickname);
Boolean db_update_user_name(DBInfo* db, int uid, const char* new_name);

int get_next_uid(DBInfo* manager);
#endif // DB_MANAGER_H