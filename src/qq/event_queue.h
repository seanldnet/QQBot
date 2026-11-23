#ifndef QQBOT_EVENT_QUEUE_H
#define QQBOT_EVENT_QUEUE_H
#include <sqlite3.h>

/* Caller serializes access to db. Returns 1 when inserted, 0 for duplicate,
 * and -1 on database/argument error. */
int qq_event_queue_init(sqlite3 *db);
int qq_event_queue_enqueue(sqlite3 *db, const char *dedupe_key,
                           const char *event_type, const char *event_json);

#endif
