#ifndef QQBOT_EVENT_QUEUE_H
#define QQBOT_EVENT_QUEUE_H
#include <sqlite3.h>

/* Caller serializes access to db. */
int qq_event_queue_init(sqlite3 *db);
/* 1 inserted, 0 duplicate, -1 error. */
int qq_event_queue_enqueue(sqlite3 *db, const char *dedupe_key,
                           const char *event_type, const char *event_json);
/* Create one pending delivery per enabled plugin with a non-empty callback.
 * Returns number newly created (0 or greater), -1 on error.
 */
int qq_event_queue_fanout(sqlite3 *db, const char *dedupe_key);

#endif
