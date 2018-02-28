
#include "dslink/message_utils.h"
#include <string.h>

#define LOG_TAG "utils"
#include <dslink/log.h>

char* dumpIndentLevels(char* buf, uint16_t indent)
{
    if(indent > 0) {
        size_t n = 0;
        for(; n < indent; ++n) {
            if(n % 2) {
                buf += sprintf(buf, " ");
            } else {
                buf += sprintf(buf, "|");
            }
        }
    }
    return buf;
}

char* dumpMessageInternal(char* buf, json_t* message, uint16_t indent)
{
    buf = dumpIndentLevels(buf, indent);

    switch(message->type) {
        case JSON_ARRAY: {
            buf += sprintf(buf, "|--> (%lu) Array:", message->refcount);

            size_t index = 0;
            json_t *value = NULL;
            json_array_foreach(message, index, value) {
                buf += sprintf(buf, "\n");
                buf = dumpMessageInternal(buf, value, indent+2);
            }
            break;
        }
        case JSON_NULL:
            buf += sprintf(buf, "|--> (%lu) NULL", message->refcount);
            break;
        case JSON_REAL:
            buf += sprintf(buf, "|--> (%lu) %f", message->refcount, json_real_value(message));
            break;
        case JSON_TRUE:
            buf += sprintf(buf, "|--> (%lu) TRUE", message->refcount);
            break;
        case JSON_FALSE:
            buf += sprintf(buf, "|--> (%lu) FALSE", message->refcount);
            break;
        case JSON_OBJECT: {
            buf += sprintf(buf, "|--> (%lu) Object:", message->refcount);
            const char *key;
            json_t *value;
            json_object_foreach(message, key, value) {
                buf += sprintf(buf, "\n");
                buf = dumpIndentLevels(buf, indent+2);
                buf += sprintf(buf, "|--> (%lu) \"%s\":\n", message->refcount, key);
                buf = dumpMessageInternal(buf, value, indent+4);
            }
            break;
        }
        case JSON_STRING:
            buf += sprintf(buf, "|--> (%lu) \"%s\"", message->refcount, json_string_value(message));
            break;
        case JSON_INTEGER:
            buf += sprintf(buf, "|--> (%lu) %lli", message->refcount, json_integer_value(message));
            break;
    }

    return buf;
}

char* dumpMessage(json_t* message)
{
    char buf[16384] = "{}";

    dumpMessageInternal(buf, message, 0);

    return strdup(buf);
}


json_t* merge_queue_messages(Vector* send_queue, uint32_t count)
{
    json_t* top = json_object();
    json_t* reqs = NULL;
    json_t *resps = NULL;

    uint32_t processed = 0;
    uint32_t msgId = 0;
    uint32_t ackId = 0;
    dslink_vector_foreach(send_queue) {
        if(processed == count) {
            break;
        }

        json_t* obj = (json_t*)(*(void**)data);

        /*
        if (dslink_log_lvl >= LOG_LVL_DEBUG) {
            char* s = dumpMessage(obj);
            log_debug("Message before: \n%s\n", s);
        }*/

        json_t* msg = json_object_get(obj, "msg");
        if(msg) {
            uint32_t tmp = (uint32_t)json_integer_value(msg);
            msgId = tmp > msgId ? tmp : msgId;
        }

        json_t* ack = json_object_get(obj, "ack");
        if(ack) {
            uint32_t tmp = (uint32_t)json_integer_value(ack);
            ackId = tmp > ackId ? tmp : ackId;
        }

        json_t* req = json_object_get(obj, "requests");
        if(req) {
            if(!reqs) {
                reqs = json_incref(req);
            } else {
                size_t index = 0;
                json_t *value = NULL;
                json_array_foreach(req, index, value) {
                    json_array_append(reqs, value);
                }
            }
        }

        json_t* resp = json_object_get(obj, "responses");
        if(resp) {
            if(!resps) {
                resps = json_incref(resp);
            } else {
                size_t index = 0;
                json_t *value = NULL;
                json_array_foreach(resp, index, value) {
                    json_array_append(resps, value);
                }
            }
        }

        json_decref(obj);
        ++processed;
    }
    dslink_vector_foreach_end();
    vector_erase_range(send_queue, 0, processed);

    if(msgId > 0) {
        json_object_set_new_nocheck(top, "msg", json_integer(msgId));
    }
    if(ackId > 0) {
        json_object_set_new_nocheck(top, "ack", json_integer(ackId));
    }
    if(reqs) {
        json_object_set_new_nocheck(top, "requests", reqs);
    }
    if(resps) {
        json_object_set_new_nocheck(top, "responses", resps);
    }

    /*
    if (dslink_log_lvl >= LOG_LVL_DEBUG) {
        char* s = dumpMessage(top);
        log_debug("Merged message: \n%s\n", s);
    }*/

    return top;
}
