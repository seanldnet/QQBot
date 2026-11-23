#include "../src/qq/event_queue.h"
#include <stdio.h>
#include <stdlib.h>

static int scalar(sqlite3 *db, const char *sql) {
    sqlite3_stmt *st = NULL;
    int value = -1;
    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) == SQLITE_OK &&
        sqlite3_step(st) == SQLITE_ROW) value = sqlite3_column_int(st, 0);
    sqlite3_finalize(st);
    return value;
}

int main(void) {
    sqlite3 *db = NULL;
    int failed = 0;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) return EXIT_FAILURE;
    sqlite3_exec(db, "PRAGMA foreign_keys=ON;"
        "CREATE TABLE plugins(id TEXT PRIMARY KEY,callback_url TEXT NOT NULL,enabled INTEGER NOT NULL);"
        "INSERT INTO plugins VALUES('p1','https://plugin-one.example/hook',1);"
        "INSERT INTO plugins VALUES('p2','https://plugin-two.example/hook',1);"
        "INSERT INTO plugins VALUES('p3','',1);"
        "INSERT INTO plugins VALUES('p4','https://disabled.example/hook',0);",
        NULL, NULL, NULL);
    if (qq_event_queue_init(db) != 0) failed = 1;
    if (qq_event_queue_enqueue(db, "GROUP_AT_MESSAGE_CREATE|m-1",
        "GROUP_AT_MESSAGE_CREATE", "{\"msg_id\":\"m-1\"}") != 1) failed = 1;
    if (qq_event_queue_enqueue(db, "GROUP_AT_MESSAGE_CREATE|m-1",
        "GROUP_AT_MESSAGE_CREATE", "{\"msg_id\":\"m-1\"}") != 0) failed = 1;
    if (qq_event_queue_enqueue(db, "", "GROUP_ADD_ROBOT", "{}") != -1) failed = 1;
    if (qq_event_queue_fanout(db, "GROUP_AT_MESSAGE_CREATE|m-1") != 2) failed = 1;
    if (qq_event_queue_fanout(db, "GROUP_AT_MESSAGE_CREATE|m-1") != 0) failed = 1;
    if (scalar(db, "SELECT count(*) FROM event_deliveries WHERE state='pending'") != 2) failed = 1;
    if (scalar(db, "SELECT count(*) FROM event_deliveries WHERE plugin_id IN ('p3','p4')") != 0) failed = 1;
    if (qq_event_queue_enqueue(db, "GROUP_ADD_ROBOT|g|u|1",
        "GROUP_ADD_ROBOT", "{\"group\":\"g\"}") != 1) failed = 1;
    if (scalar(db, "SELECT count(*) FROM event_inbox WHERE state='pending'") != 2) failed = 1;
    sqlite3_close(db);
    if (failed) fprintf(stderr, "event queue/fanout test failed\n");
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
