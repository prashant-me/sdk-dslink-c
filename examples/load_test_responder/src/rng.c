#define LOG_TAG "rng"

#include <dslink/log.h>
#include "rng.h"

struct RequesterLink {
    DSLink *link;
    DSNode *node;
    int current_sequence_number;
};

static
void gen_number(uv_timer_t *timer) {
    struct RequesterLink *rl = timer->data;
    DSLink *link = rl->link;
    DSNode *node = rl->node;

    if (!dslink_map_contains(link->responder->value_path_subs,
                             (void *) node->path)) {
        dslink_free(rl);
        uv_timer_stop(timer);
        return;
    }

    if(rl->current_sequence_number == 1) {
        dslink_node_set_value(link, node, json_integer(rl->current_sequence_number));
    } else {
        dslink_node_update_value_new(link, node, json_integer(rl->current_sequence_number));
    }
    rl->current_sequence_number++;
}

static
void responder_rng_subbed(DSLink *link, DSNode *node) {
    log_info("Subscribed to %s\n", node->path);

    struct RequesterLink *rl = malloc(sizeof(struct RequesterLink));
    rl->link = link;
    rl->node = node;

    // Each broker connecting to this link will
    // get a fresh sequence number.
    rl->current_sequence_number = 1;

    uv_timer_t *timer = malloc(sizeof(uv_timer_t));
    uv_timer_init(&link->loop, timer);
    timer->data = rl;
    uv_timer_start(timer, gen_number, 0, 1);
}

static
void responder_rng_unsubbed(DSLink *link, DSNode *node) {
    (void) link;
    log_info("Unsubscribed to %s\n", node->path);
}

void responder_init_rng(DSLink *link, DSNode *root) {
    DSNode *num = dslink_node_create(root, "rng", "node");
    if (!num) {
        log_warn("Failed to create the rng node\n");
        return;
    }

    num->on_subscribe = responder_rng_subbed;
    num->on_unsubscribe = responder_rng_unsubbed;
    if (dslink_node_set_meta(link, num, "$type", json_string("number")) != 0) {
        log_warn("Failed to set the type on the rng\n");
        dslink_node_tree_free(link, num);
        return;
    }

    if (dslink_node_add_child(link, num) != 0) {
        log_warn("Failed to add the rng node to the root\n");
        dslink_node_tree_free(link, num);
    }
}
