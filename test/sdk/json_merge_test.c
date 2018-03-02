#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dslink/message_utils.h"

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

    json_t* top = merge_queue_messages(&send_queue, 10);
    assert_int_equal(0, vector_count(&send_queue));
    vector_free(&send_queue);

    json_t* reqs = json_object_get(top, "requests");
    assert_non_null(reqs);
    assert_int_equal(4u, json_array_size(reqs));

    json_t* resps = json_object_get(top, "responses");
    assert_non_null(resps);
    assert_int_equal(2u, json_array_size(resps));

    char* result = json_dumps(top, JSON_PRESERVE_ORDER);
    json_decref(top);

    const char* expected = "{\"requests\": [{\"method\": \"close\", \"rid\": 2913}, {\"method\": \"close\", \"rid\": 2912}, {\"method\": \"close\", \"rid\": 2911}, {\"method\": \"close\", \"rid\": 2907}], \"responses\": [{\"stream\": \"open\", \"rid\": 1646, \"updates\": [[\"testnode_00121\", {\"$is\": \"node\", \"$name\": \"testnode_00121\"}]]}, {\"stream\": \"open\", \"rid\": 1646, \"updates\": [[\"testnode_00122\", {\"$is\": \"node\", \"$name\": \"testnode_00122\"}]]}]}";

    assert_string_equal(expected, result);
}

static
void json_merge_count_test(void **state) {
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

    json_t* top = merge_queue_messages(&send_queue, 4);
    json_decref(top);
    assert_int_equal(2, vector_count(&send_queue));
    top = merge_queue_messages(&send_queue, 4);
    json_decref(top);
    assert_int_equal(0, vector_count(&send_queue));
    vector_free(&send_queue);
}


int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(json_merge_test),
        cmocka_unit_test(json_merge_count_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
