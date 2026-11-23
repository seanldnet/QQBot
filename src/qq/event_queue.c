#include "event_queue.h"
#include <string.h>

int qq_event_queue_init(sqlite3 *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS event_inbox("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "dedupe_key TEXT NOT NULL UNIQUE,"
        "event_type TEXT NOT NULL,"
        "event_json TEXT NOT NULL,"
        "state TEXT NOT NULL DEFAULT 'pending',"
        "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_event_inbox_state_id ON event_inbox(state,id);";
    return db && sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK ? 0 : -1;
}

int qq_event_queue_enqueue(sqlite3 *db, const char *dedupe_key,
                           const char *event_type, const char *event_json) {
    sqlite3_stmt *st = NULL;
    int rc;
    if (!db || !dedupe_key || !dedupe_key[0] || !event_type ||
        !event_type[0] || !event_json) return -1;
    rc = sqlite3_prepare_v2(db,
        "INSERT OR IGNORE INTO event_inbox(dedupe_key,event_type,event_json) VALUES(?1,?2,?3)",
        -1, &st, NULL);
    if (rc != SQLITE_OK) return -1;
    sqlite3_bind_text(st, 1, dedupe_key, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 2, event_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 3, event_json, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) return -1;
    return sqlite3_changes(db) == 1 ? 1 : 0;
}
