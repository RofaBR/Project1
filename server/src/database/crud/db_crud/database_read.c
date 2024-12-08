#include "../../../../inc/server.h"

static int callback(void* data, int argc, char** argv, char** azColName) {
    t_list** list = (t_list**)data;
    for (int i = 0; i < argc; i++) {
        if (mx_strlen(argv[i]) != 0) {
            mx_push_back(*(&list), mx_strdup(argv[i]));
        }
        else {
            mx_push_back(*(&list), mx_strdup("NULL"));
        }

    }
    return 0;
}

t_list *database_read(const char* select, const char* from, const char* where) {
    sqlite3* db;
    char* error = NULL;
    char* command = NULL;
    t_list* res = NULL;
    int rc = sqlite3_open(exe_path, &db);

    if(!validate_database_operation(rc, db, NULL)) return NULL;

    if (where == NULL) {
        asprintf(&command, "SELECT %s FROM %s", select, from);
    }
    else {
        asprintf(&command, "SELECT %s FROM %s WHERE %s", select, from, where);
    }

    rc = sqlite3_exec(db, command, callback, &res, &error);

    if(!validate_database_operation(rc, db, error)) return NULL;

    mx_strdel(&command);
    sqlite3_close(db);

    return res;
}
