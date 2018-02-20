#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jansson.h>
#include "dslink/col/vector.h"

#include "cmocka_init.h"

json_t* loadJson(const char* s)
{
    json_error_t err;
    return json_loadb(s, strlen(s), JSON_PRESERVE_ORDER, &err);
}


static
void json_merge_test(void **state) {
    (void) state;

    Vector send_queue;
    vector_init(&send_queue, 10, sizeof(json_t*));

    {
        const char* r = "{\"requests\":[{\"method\":\"close\",\"rid\":2913},{\"method\":\"close\",\"rid\":2912},{\"method\":\"close\",\"rid\":2911},{\"method\":\"close\",\"rid\":2907}]}";
        json_t* req = loadJson(r);
        vector_append(&send_queue, &req);
    }

    {
        const char* r = "{\"responses\": [{\"stream\": \"open\", \"rid\": 1646, \"updates\": [[\"testnode_00121\", {\"$is\": \"node\", \"$name\": \"testnode_00121\"}]]}]}";
        json_t* resp = loadJson(r);
        vector_append(&send_queue, &resp);
    }

    {
        const char* r = "{\"responses\": [{\"stream\": \"open\", \"rid\": 1646, \"updates\": [[\"testnode_00122\", {\"$is\": \"node\", \"$name\": \"testnode_00122\"}]]}]}";
        json_t* resp = loadJson(r);
        vector_append(&send_queue, &resp);
    }

    json_t* top = json_object();
    json_t* reqs = NULL;
    json_t *resps = NULL;

    dslink_vector_foreach(&send_queue) {
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
    }
    dslink_vector_foreach_end();
    vector_free(&send_queue);

    char* s = json_dumps(top, JSON_PRESERVE_ORDER);
    printf("%s\n", s);
    json_decref(top);
}


int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(json_merge_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
