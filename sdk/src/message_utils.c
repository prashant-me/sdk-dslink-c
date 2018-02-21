
#include "dslink/message_utils.h"


json_t* merge_queue_messages(Vector* send_queue, uint32_t count)
{
    json_t* top = json_object();
    json_t* reqs = NULL;
    json_t *resps = NULL;

    uint32_t processed = 0;
    dslink_vector_foreach(send_queue) {
        if(processed == count) {
            break;
        }

        json_t* obj = (json_t*)(*(void**)data);

        json_t* req = json_object_get(obj, "requests");
        if(!reqs) {
            reqs = req;
            json_object_set(top, "requests", json_incref(reqs));
        } else {
            size_t index = 0;
            json_t *value = NULL;
            json_array_foreach(req, index, value) {
                json_array_append(reqs, json_incref(value));
            }
        }

        json_t* resp = json_object_get(obj, "responses");
        if(!resps) {
            resps = resp;
            json_object_set(top, "responses", json_incref(resps));
        } else {
            size_t index = 0;
            json_t *value = NULL;
            json_array_foreach(resp, index, value) {
                json_array_append(resps, json_incref(value));
            }
        }

        json_decref(obj);
        ++processed;
    }
    dslink_vector_foreach_end();
    vector_erase_range(send_queue, 0, processed);

    return top;
}
