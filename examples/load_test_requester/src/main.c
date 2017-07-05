#define LOG_TAG "main"

#include <dslink/log.h>
#include <dslink/requester.h>

#include <stdio.h>

void on_req_close(struct DSLink *link, ref_t *req_ref, json_t *resp) {
    (void) link;
    (void) resp;
    (void) req_ref;
    (void) resp;
}

void configure_request(ref_t *ref) {
    RequestHolder *req = ref->data;
    req->close_cb = on_req_close;
}

void on_invoke_updates(struct DSLink *link, ref_t *req_ref, json_t *resp) {
    (void) link;
    (void) req_ref;
    char *data = json_dumps(resp, JSON_INDENT(2));
    printf("Got invoke %s\n", data);
    dslink_free(data);
}

ref_t *streamInvokeRef = NULL;
void start_stream_invoke(DSLink *link) {
    json_t *params = json_object();
    json_object_set_new(params, "Path", json_string("/data/test_c_sdk"));
    json_object_set_new(params, "Value", json_integer(-1));
    streamInvokeRef = dslink_requester_invoke(
            link,
            "/data/publish",
            params,
            on_invoke_updates
    );
    json_decref(params);
    configure_request(streamInvokeRef);
}

void on_list_update(struct DSLink *link, ref_t *req_ref, json_t *resp) {
    (void) link;
    RequestHolder *holder = req_ref->data;

    json_t *updates = json_object_get(resp, "updates");
    size_t index;
    json_t *value;

    const char* path = json_string_value(json_object_get(holder->req, "path"));

    printf("======= List %s =======\n", path);
    json_array_foreach(updates, index, value) {
        json_t *name = json_array_get(value, 0);
        json_t *val = json_array_get(value, 1);

        if (val->type == JSON_ARRAY || val->type == JSON_OBJECT) {
            char *data = json_dumps(val, JSON_INDENT(0));
            printf("%s = %s\n", json_string_value(name), data);
            dslink_free(data);
        } else if (val->type == JSON_STRING) {
            printf("%s = %s\n", json_string_value(name), json_string_value(val));
        } else if (val->type == JSON_INTEGER) {
            printf("%s = %lli\n", json_string_value(name), json_integer_value(val));
        } else if (val->type == JSON_REAL) {
            printf("%s = %f\n", json_string_value(name), json_real_value(val));
        } else if (val->type == JSON_NULL) {
            printf("%s = NULL\n", json_string_value(name));
        } else if (val->type == JSON_TRUE) {
            printf("%s = true\n", json_string_value(name));
        } else if (val->type == JSON_FALSE) {
            printf("%s = false\n", json_string_value(name));
        } else {
            printf("%s = (Unknown Type)\n", json_string_value(name));
        }
    }

    dslink_requester_close(link, (uint32_t) json_integer_value(json_object_get(resp, "rid")));
}

void on_value_update(struct DSLink *link, uint32_t sid, json_t *val, json_t *ts) {
    (void) link;
    (void) ts;
    (void) sid;

    static int lastValue = 0;
    const int currentValue = (int)json_integer_value(val);
    if(++lastValue != currentValue) {
        printf("ERROR: Got value %d, expected value %d\n", (int)json_integer_value(val), lastValue);
    }

    if(currentValue%1000 == 0) {
        log_info("Received %d messages\n", (int)json_integer_value(val));
    }

}

void init(DSLink *link) {
    (void) link;
    log_info("Initialized %s!\n", link->config.name);
}

void connected(DSLink *link) {
    (void) link;
    log_info("Connected!\n");
}

void disconnected(DSLink *link) {
    (void) link;
    log_info("Disconnected!\n");
}

void requester_ready(DSLink *link) {
    char buf[1024];
    sprintf(buf, "%s.requestpath", link->config.name);

    FILE* fp = fopen(buf, "r");
    if (NULL == fp) {
        log_info("Could not open file %s'", buf);
        return;
    }

    char subscriptionPath[2048];
    fgets(subscriptionPath, sizeof(subscriptionPath), fp);
    fclose(fp);

    sprintf(buf, "/downstream/responder%d/rng", link_id);
    configure_request(dslink_requester_list(link, "/downstream", on_list_update));
    configure_request(dslink_requester_subscribe(
        link,
        subscriptionPath,
        on_value_update,
        0
    ));

    start_stream_invoke(link);
}

int main(int argc, char **argv) {
    DSLinkCallbacks cbs = {
        init,
        connected,
        disconnected,
        requester_ready
    };

    return dslink_init(argc, argv, "C-Load-Test-Requester", 1, 0, &cbs);
}
