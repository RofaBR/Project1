#include "../../../../inc/server.h"

static int callback(void* data, int argc, char** argv, char** azColName) {
    t_list** list = (t_list**)data;
    for (int i = 0; i < argc; i++) {
        mx_push_back(*(&list), mx_strdup(argv[i]));
    }
    return 0;
}

t_list *database_read(const char* select, const char* from, const char* where) {
    sqlite3* db;
    char* error = NULL;
    char* command = NULL;
    t_list* res = NULL;
    int rc = sqlite3_open(DB_NAME, &db);

    validate_database_operation(rc, db, NULL);

    if (where == NULL) {
        asprintf(&command, "SELECT %s FROM %s", select, from);
    }
    else {
        asprintf(&command, "SELECT %s FROM %s WHERE %s", select, from, where);
    }
    rc = sqlite3_exec(db, command, callback, &res, &error);
    validate_database_operation(rc, db, error);

    mx_strdel(&command);
    sqlite3_close(db);

    return res;
}