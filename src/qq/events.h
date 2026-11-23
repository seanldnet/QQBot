#ifndef QQBOT_EVENTS_H
#define QQBOT_EVENTS_H

/* Normalize a QQ Gateway Dispatch payload into the plugin event schema.
 * event_type is the Gateway dispatch name (the outer "t" field).
 * data_json is the serialized inner "d" object.
 * On success, *normalized_json is heap-allocated and must be free()'d.
 * Returns 0 on success, -1 for invalid input, -2 for unsupported event.
 */
int qq_event_normalize(const char *event_type, const char *data_json,
                       char **normalized_json);

#endif
