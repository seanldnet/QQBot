#include "event_queue.h"
#include <stddef.h>

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
        "CREATE INDEX IF NOT EXISTS idx_event_inbox_state_id ON event_inbox(state,id);"
        "CREATE TABLE IF NOT EXISTS event_deliveries("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "event_id INTEGER NOT NULL REFERENCES event_inbox(id) ON DELETE CASCADE,"
        "plugin_id TEXT NOT NULL REFERENCES plugins(id) ON DELETE CASCADE,"
        "callback_url TEXT NOT NULL,"
        "state TEXT NOT NULL DEFAULT 'pending',"
        "attempts INTEGER NOT NULL DEFAULT 0,"
        "last_error TEXT NOT NULL DEFAULT '',"
        "next_attempt_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE(event_id,plugin_id)"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_event_deliveries_state_next ON event_deliveries(state,next_attempt_at,id);";
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

int qq_event_queue_fanout(sqlite3 *db, const char *dedupe_key) {
    sqlite3_stmt *st = NULL;
    int rc;
    if (!db || !dedupe_key || !dedupe_key[0]) return -1;
    rc = sqlite3_prepare_v2(db,
        "INSERT OR IGNORE INTO event_deliveries(event_id,plugin_id,callback_url) "
        "SELECT e.id,p.id,p.callback_url FROM event_inbox e "
        "JOIN plugins p ON p.enabled=1 AND p.callback_url<>'' "
        "WHERE e.dedupe_key=?1",
        -1, &st, NULL);
    if (rc != SQLITE_OK) return -1;
    sqlite3_bind_text(st, 1, dedupe_key, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) return -1;
    return sqlite3_changes(db);
}
