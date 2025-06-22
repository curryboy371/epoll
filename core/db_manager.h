#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include "define.h"
#include <mongoc/mongoc.h>

typedef struct {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *user_collection;
} DBInfo;

Boolean db_init(DBInfo *manager, const char *uri, const char *dbname);
void db_cleanup(DBInfo *manager);

Boolean db_find_user(DBInfo *manager, const char *user_id, char *password_out, size_t out_len);
Boolean db_insert_user(DBInfo *manager, const char *user_id, const char *password, const char *nickname);

#endif // DB_MANAGER_H