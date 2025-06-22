#include "db_manager.h"


Boolean db_init(DBInfo *manager, const char *uri, const char *dbname)
{
    mongoc_init();

    manager->client = mongoc_client_new(uri);
    if (!manager->client) {
        printf("MongoDB client create failed\n");
        return FALSE;
    }

    manager->database = mongoc_client_get_database(manager->client, dbname);
    manager->user_collection = mongoc_client_get_collection(manager->client, dbname, "users");

    return TRUE;
}

void db_cleanup(DBInfo *manager)
{
    if (manager->user_collection)
        mongoc_collection_destroy(manager->user_collection);
    if (manager->database)
        mongoc_database_destroy(manager->database);
    if (manager->client)
        mongoc_client_destroy(manager->client);

    mongoc_cleanup();
}

Boolean db_find_user(DBInfo *manager, const char *user_id, char *password_out, size_t out_len)
{
    bson_t *query = BCON_NEW("id", BCON_UTF8(user_id));
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(manager->user_collection, query, NULL, NULL);
    const bson_t *doc;
    Boolean found = FALSE;

    while (mongoc_cursor_next(cursor, &doc))
    {
        bson_iter_t iter;
        if (bson_iter_init_find(&iter, doc, "password") && BSON_ITER_HOLDS_UTF8(&iter))
        {
            const char *pw = bson_iter_utf8(&iter, NULL);
            strncpy(password_out, pw, out_len);
            found = TRUE;
            break;
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    return found;
}

Boolean db_insert_user(DBInfo *manager, const char *user_id, const char *password, const char *nickname)
{
    // 먼저 중복검사
    char dummy_pw[256];
    if (db_find_user(manager, user_id, dummy_pw, sizeof(dummy_pw)))
    {
        printf("이미 존재하는 ID입니다: %s\n", user_id);
        return FALSE;
    }

    // 새 문서 생성
    bson_t *new_doc = BCON_NEW(
        "id", BCON_UTF8(user_id),
        "password", BCON_UTF8(password),
        "name", BCON_UTF8(nickname)
    );

    bson_error_t error;
    if (!mongoc_collection_insert_one(manager->user_collection, new_doc, NULL, NULL, &error))
    {
        printf("MongoDB Insert 실패: %s\n", error.message);
        bson_destroy(new_doc);
        return FALSE;
    }

    bson_destroy(new_doc);
    printf("회원가입 성공: %s\n", user_id);
    return TRUE;
}