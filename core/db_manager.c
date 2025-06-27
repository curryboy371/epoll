#include "db_manager.h"


Boolean db_init(DBInfo *manager, const char *uri, const char *dbname)
{
    mongoc_init();

    manager->client = mongoc_client_new(uri);
    if (!manager->client) {
        printf("MongoDB client create failed\n");
        return FALSE;
    }


    // 연결 테스트 (ping)
    const bson_t *ping_command;
    bson_t reply;
    bson_error_t error;

    ping_command = BCON_NEW("ping", BCON_INT32(1));

    if (!mongoc_client_command_simple(manager->client, "admin", ping_command, NULL, &reply, &error)) {
        printf("MongoDB ping failed: %s\n", error.message);
        bson_destroy(&reply);
        bson_destroy((bson_t*) ping_command);
        return FALSE;
    }

    bson_destroy(&reply);
    bson_destroy((bson_t*) ping_command);


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
    Boolean found = FALSE;

    while (mongoc_cursor_next(cursor, &doc))
    {
        bson_iter_t iter;
        if (bson_iter_init(&iter, doc))
        {
            while (bson_iter_next(&iter))
            {
                const char *key = bson_iter_key(&iter);
                if (strcmp(key, "id") == 0 && BSON_ITER_HOLDS_UTF8(&iter))
                {
                    const char *id_value = bson_iter_utf8(&iter, NULL);
                    if (strcmp(id_value, user_id) == 0)
                    {
                        found = TRUE;
                        break;
                    }
                }
            }
        }

        if (found)
            break;
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    return found;
}

Boolean db_find_user_by_pw(DBInfo *manager, const char *user_id, const char *password, User* out_user)
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
            int uid_in_db = -1;

            while (bson_iter_next(&iter)) 
            {
                const char *key = bson_iter_key(&iter);

                if (strcmp(key, "password") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
                    pw_in_db = bson_iter_utf8(&iter, NULL);
                }
                else if (strcmp(key, "name") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
                    name_in_db = bson_iter_utf8(&iter, NULL);
                }
                else if (strcmp(key, "uid") == 0 && BSON_ITER_HOLDS_INT32(&iter)) {
                    uid_in_db = bson_iter_int32(&iter);
                }
            }

            if (pw_in_db == NULL || name_in_db == NULL || uid_in_db  == 0) {
                continue; 
            }

            // 패스워드 비교
            if (strcmp(password, pw_in_db) != 0) {
                continue; // 비밀번호 불일치시 다음 doc
            }

            // 매칭 성공
            strncpy(out_user->id, user_id, sizeof(out_user->id) - 1);
            out_user->id[sizeof(out_user->id) - 1] = '\0';

            strncpy(out_user->name, name_in_db, sizeof(out_user->name) - 1);
            out_user->name[sizeof(out_user->name) - 1] = '\0';

            out_user->uid = uid_in_db;

            found = TRUE;
            break;
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    return found;
}

Boolean db_find_all_users(DBInfo *manager, User* out_users, size_t* out_count)
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
            strncpy(out_users[count].id, id_in_db, sizeof(out_users[count].id) - 1);
            out_users[count].id[sizeof(out_users[count].id) - 1] = '\0';

            strncpy(out_users[count].name, name_in_db, sizeof(out_users[count].name) - 1);
            out_users[count].name[sizeof(out_users[count].name) - 1] = '\0';

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
    int uid = get_next_uid(manager);
    if (uid < 0) {
        printf("UID 생성 실패!\n");
        return FALSE;
    }

    // 중복검사
    if (db_find_user_by_id(manager, user_id))
    {
        printf("이미 존재하는 ID입니다: %s\n", user_id);
        return FALSE;
    }

    // 
    bson_t *new_doc = BCON_NEW(
        "id", BCON_UTF8(user_id),
        "password", BCON_UTF8(password),
        "name", BCON_UTF8(nickname),
        "uid", BCON_INT32(uid)
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

Boolean db_update_user_name(DBInfo* db, int uid, const char* new_name) {
    bson_t* query = BCON_NEW("uid", BCON_INT32(uid));
    bson_t* update = BCON_NEW("$set", "{", "name", BCON_UTF8(new_name), "}");

    bson_error_t error;

    bool result = mongoc_collection_update_one(
        db->user_collection,
        query,
        update,
        NULL,
        NULL,
        &error
    );

    if (!result) {
        printf("DB name update failed: %s\n", error.message);
    }

    bson_destroy(query);
    bson_destroy(update);


    return result ? TRUE : FALSE;
}

int get_next_uid(DBInfo* manager)
{
    bson_t filter;
    bson_init(&filter);  // 빈 필터 {}

    bson_error_t error;
    int64_t count = mongoc_collection_count_documents(
        manager->user_collection, 
        &filter, 
        NULL,  // opts
        NULL,  // read prefs
        NULL,  // read concern
        &error     // error (반드시 필요)
    );

    bson_destroy(&filter);  // 사용 후 반드시 해제

    if (count < 0) {
        printf("MongoDB count 실패\n");
        return -1;
    }

    return (int)(count + 1000);
}