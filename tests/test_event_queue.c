#include "../src/qq/event_queue.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    sqlite3 *db = NULL;
    int failed = 0;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) return EXIT_FAILURE;
    if (qq_event_queue_init(db) != 0) failed = 1;
    if (qq_event_queue_enqueue(db, "GROUP_AT_MESSAGE_CREATE|m-1",
                               "GROUP_AT_MESSAGE_CREATE", "{\"msg_id\":\"m-1\"}") != 1) failed = 1;
    if (qq_event_queue_enqueue(db, "GROUP_AT_MESSAGE_CREATE|m-1",
                               "GROUP_AT_MESSAGE_CREATE", "{\"msg_id\":\"m-1\"}") != 0) failed = 1;
    if (qq_event_queue_enqueue(db, "", "GROUP_ADD_ROBOT", "{}") != -1) failed = 1;
    if (qq_event_queue_enqueue(db, "GROUP_ADD_ROBOT|g|u|1",
                               "GROUP_ADD_ROBOT", "{\"group\":\"g\"}") != 1) failed = 1;
    sqlite3_stmt *st = NULL;
    if (sqlite3_prepare_v2(db, "SELECT count(*) FROM event_inbox WHERE state='pending'", -1, &st, NULL) != SQLITE_OK ||
        sqlite3_step(st) != SQLITE_ROW || sqlite3_column_int(st, 0) != 2) failed = 1;
    sqlite3_finalize(st);
    sqlite3_close(db);
    if (failed) fprintf(stderr, "event queue test failed\n");
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
