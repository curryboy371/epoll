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

Boolean db_find_user_by_id(DBInfo *manager, const char *user_id)
{
    bson_t *query = BCON_NEW("id", BCON_UTF8(user_id));
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(manager->user_collection, query, NULL, NULL);
    const bson_t *doc;

    while (mongoc_cursor_next(cursor, &doc))
    {
       bson_iter_t iter;
        if (bson_iter_init(&iter, doc)) 
        {
            // 전체 필드 순회
            while (bson_iter_next(&iter)) 
            {
                const char *key = bson_iter_key(&iter);
                if (strcmp(user_id, "id") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) 
                {
                    return TRUE;
                }
            }
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    return FALSE;
}


Boolean db_find_user_by_pw(DBInfo *manager, const char *user_id, const char *password, User_Data* out_user)
{
    bson_t *query = BCON_NEW("id", BCON_UTF8(user_id));
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(manager->user_collection, query, NULL, NULL);
    const bson_t *doc;

    Boolean found = FALSE;
    while (mongoc_cursor_next(cursor, &doc))
    {
       bson_iter_t iter;
        if (bson_iter_init(&iter, doc)) 
        {
            const char *pw_in_db = NULL;
            const char *name_in_db = NULL;

            while (bson_iter_next(&iter)) 
            {
                const char *key = bson_iter_key(&iter);

                if (strcmp(key, "password") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
                    pw_in_db = bson_iter_utf8(&iter, NULL);
                }
                else if (strcmp(key, "name") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
                    name_in_db = bson_iter_utf8(&iter, NULL);
                }
            }

            if (pw_in_db == NULL || name_in_db == NULL) {
                continue;  // 패스워드나 이름이 없는 경우 skip
            }

            // 패스워드 비교
            if (strcmp(password, pw_in_db) != 0) {
                continue; // 비밀번호 불일치시 다음 doc
            }

            // 매칭 성공
            strncpy(out_user->user_id, user_id, sizeof(out_user->user_id) - 1);
            out_user->user_id[sizeof(out_user->user_id) - 1] = '\0';

            strncpy(out_user->user_name, name_in_db, sizeof(out_user->user_name) - 1);
            out_user->user_name[sizeof(out_user->user_name) - 1] = '\0';

            found = TRUE;
            break;
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    return found;
}

Boolean db_find_all_users(DBInfo *manager, User_Data* out_users, size_t* out_count)
{
    bson_t *query = bson_new();  // 전체 검색
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(manager->user_collection, query, NULL, NULL);
    const bson_t *doc;
    size_t count = 0;

    while (mongoc_cursor_next(cursor, &doc))
    {
        bson_iter_t iter;
        if (bson_iter_init(&iter, doc))
        {
            const char *id_in_db = NULL;
            const char *name_in_db = NULL;

            while (bson_iter_next(&iter))
            {
                const char *key = bson_iter_key(&iter);
                if (strcmp(key, "id") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
                    id_in_db = bson_iter_utf8(&iter, NULL);
                }
                else if (strcmp(key, "name") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
                    name_in_db = bson_iter_utf8(&iter, NULL);
                }
            }

            if (id_in_db == NULL || name_in_db == NULL) {
                continue;  // 필수 필드 없는 경우 skip
            }

            // 데이터 저장
            strncpy(out_users[count].user_id, id_in_db, sizeof(out_users[count].user_id) - 1);
            out_users[count].user_id[sizeof(out_users[count].user_id) - 1] = '\0';

            strncpy(out_users[count].user_name, name_in_db, sizeof(out_users[count].user_name) - 1);
            out_users[count].user_name[sizeof(out_users[count].user_name) - 1] = '\0';

            count++;
        }
    }

    *out_count = count;

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return count > 0 ? TRUE : FALSE;
}

Boolean db_insert_user(DBInfo *manager, const char *user_id, const char *password, const char *nickname)
{
    // 중복검사
    if (db_find_user_by_id(manager, user_id))
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