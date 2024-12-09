#include "../../../inc/server.h"

void users_migration_up(void) {
    sqlite3* db;
    char* error = NULL;
    int rc = sqlite3_open(exe_path, &db);

    if(!validate_database_operation(rc, db, NULL)) return;

    rc = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        "username VARCHAR(255) NOT NULL,"
        "login VARCHAR(255) NOT NULL UNIQUE,"
        "password VARCHAR(255) NOT NULL,"
        "logo_id INTEGER DEFAULT 1,"
        "created_at DATETIME NOT NULL)", 0, 0, &error);
    
    validate_database_operation(rc, db, error);

    sqlite3_close(db);
}

void users_migration_down(void) {
    sqlite3* db;
    char* error = NULL;
    int rc = sqlite3_open(exe_path, &db);

    if(!validate_database_operation(rc, db, NULL)) return;

    rc = sqlite3_exec(db,
        "DROP TABLE IF EXISTS users", 0, 0, &error);

    validate_database_operation(rc, db, error);

    sqlite3_close(db);
}
