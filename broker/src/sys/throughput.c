#include <broker/sys/throughput.h>

#include <broker/node.h>
#include <broker/stream.h>
#include <broker/broker.h>

static BrokerNode *messagesOutPerSecond;
static BrokerNode *dataOutPerSecond;
static BrokerNode *frameOutPerSecond;

static BrokerNode *messagesInPerSecond;
static BrokerNode *dataInPerSecond;
static BrokerNode *frameInPerSecond;

static BrokerNode *outputQueueLengthPerSecond;
static BrokerNode *avgParsingTimePerSecond;
static BrokerNode *maxParsingTime;

static int outframes = -1;
static int outbytes = 0;
static int outmessages = 0;

static int inframes = -1;
static int inbytes = 0;
static int inmessages = 0;
static int inparsingtime = 0;
static int inmaxparsingtime = 0;

static uv_timer_t throughputTimer;

static void onThroughputTimer(uv_timer_t *handle) {
    (void) handle;
    if (inframes >= 0) {
        int t = inframes;
        broker_node_update_value(frameInPerSecond, json_integer(t), 1);

        t = inbytes; inbytes = 0;
        broker_node_update_value(dataInPerSecond, json_integer(t), 1);

        t = inmessages; inmessages = 0;
        broker_node_update_value(messagesInPerSecond, json_integer(t), 1);

        broker_node_update_value(maxParsingTime, json_integer(inmaxparsingtime), 1);

        if(inframes > 0) {
            t = inparsingtime / inframes; inparsingtime = 0;
            broker_node_update_value(avgParsingTimePerSecond, json_integer(t), 1);
        }
        inframes = 0;
    }
    if (outframes >= 0) {
        int t = outframes; outframes = 0;
        broker_node_update_value(frameOutPerSecond, json_integer(t), 1);

        t = outbytes; outbytes = 0;
        broker_node_update_value(dataOutPerSecond, json_integer(t), 1);

        t = outmessages; outmessages = 0;
        broker_node_update_value(messagesOutPerSecond, json_integer(t), 1);
    }
    {
        int outputQueueLength = 0;

        {
            ref_t *ref = dslink_map_get(outputQueueLengthPerSecond->parent->parent->children, "downstream");
            BrokerNode *downstream = (BrokerNode *)ref->data;

            dslink_map_foreach(downstream->children) {
                BrokerNode *child = (BrokerNode *) entry->value->data;

                if (child->type == DOWNSTREAM_NODE) {
                    DownstreamNode *downstreamNode = (DownstreamNode *) child;
                    if (downstreamNode->link) {
                        outputQueueLength += (int)(wslay_event_get_queued_msg_count(downstreamNode->link->ws));
                    }
                }
            }
        }
        {
            ref_t *ref = dslink_map_get(outputQueueLengthPerSecond->parent->parent->children, "upstream");
            BrokerNode *upstream = (BrokerNode *)ref->data;

            dslink_map_foreach(upstream->children) {
                BrokerNode *child = (BrokerNode *) entry->value->data;

                if (child->type == DOWNSTREAM_NODE) {
                    DownstreamNode *upstreamNode = (DownstreamNode *) child;
                    if (upstreamNode->link) {
                        outputQueueLength += (int)(wslay_event_get_queued_msg_count(upstreamNode->link->ws));
                    }
                }
            }
        }

        broker_node_update_value(outputQueueLengthPerSecond, json_integer(outputQueueLength), 1);
    }
}

void set_json_atttribute_no_check(json_t* meta, const char* key, json_t* value) {
    json_object_set_nocheck(meta, key, value);
    json_decref(value);
}

int init_throughput(struct BrokerNode *sysNode) {
    messagesOutPerSecond = broker_node_create("messagesOutPerSecond", "node");
    
    set_json_atttribute_no_check(messagesOutPerSecond->meta, "$type", json_string_nocheck("number"));

    broker_node_add(sysNode, messagesOutPerSecond);

    dataOutPerSecond = broker_node_create("dataOutPerSecond", "node");
    set_json_atttribute_no_check(dataOutPerSecond->meta, "$type", json_string_nocheck("number"));
    set_json_atttribute_no_check(dataOutPerSecond->meta, "@unit", json_string_nocheck("bytes"));
    broker_node_add(sysNode, dataOutPerSecond);

    frameOutPerSecond = broker_node_create("frameOutPerSecond", "node");
    set_json_atttribute_no_check(frameOutPerSecond->meta, "$type", json_string_nocheck("number"));
    broker_node_add(sysNode, frameOutPerSecond);

    messagesInPerSecond = broker_node_create("messagesInPerSecond", "node");
    set_json_atttribute_no_check(messagesInPerSecond->meta, "$type", json_string_nocheck("number"));
    broker_node_add(sysNode, messagesInPerSecond);

    dataInPerSecond = broker_node_create("dataInPerSecond", "node");
    set_json_atttribute_no_check(dataInPerSecond->meta, "$type", json_string_nocheck("number"));
    set_json_atttribute_no_check(dataInPerSecond->meta, "@unit", json_string_nocheck("bytes"));
    broker_node_add(sysNode, dataInPerSecond);

    frameInPerSecond = broker_node_create("frameInPerSecond", "node");
    set_json_atttribute_no_check(frameInPerSecond->meta, "$type", json_string_nocheck("number"));
    broker_node_add(sysNode, frameInPerSecond);

    outputQueueLengthPerSecond = broker_node_create("outputQueueLength", "node");
    set_json_atttribute_no_check(outputQueueLengthPerSecond->meta, "$type", json_string_nocheck("number"));
    broker_node_add(sysNode, outputQueueLengthPerSecond);

    avgParsingTimePerSecond = broker_node_create("avgParsingTimePerSecnd", "node");
    set_json_atttribute_no_check(avgParsingTimePerSecond->meta, "$type", json_string_nocheck("number"));
    broker_node_add(sysNode, avgParsingTimePerSecond);

    maxParsingTime = broker_node_create("maxParsingTime", "node");
    set_json_atttribute_no_check(maxParsingTime->meta, "$type", json_string_nocheck("number"));
    broker_node_add(sysNode, maxParsingTime);

    uv_timer_init(mainLoop, &throughputTimer);
    uv_timer_start(&throughputTimer, onThroughputTimer, 1000, 1000);
    return 0;
}


int throughput_input_needed() {
    if (messagesInPerSecond->sub_stream) {
        return 1;
    }
    if (dataInPerSecond->sub_stream) {
        return 1;
    }
    if (frameInPerSecond->sub_stream) {
        return 1;
    }
    if (avgParsingTimePerSecond->sub_stream) {
        return 1;
    }
    if (maxParsingTime->sub_stream) {
        return 1;
    }

    inframes = -1;
    return 0;
}

void throughput_add_input(int bytes, int messages, int parsingtime) {
    if (inframes < 0) {
        inframes = 1;
    } else {
        inframes++;
    }
    inbytes += bytes;
    inmessages += messages;
    inparsingtime += parsingtime;
    if(parsingtime > inmaxparsingtime) {
        inmaxparsingtime = parsingtime;
    }
}


int throughput_output_needed() {
    if (messagesOutPerSecond->sub_stream) {
        return 1;
    }
    if (dataOutPerSecond->sub_stream) {
        return 1;
    }
    if (frameOutPerSecond->sub_stream) {
        return 1;
    }
    if (outputQueueLengthPerSecond->sub_stream) {
        return 1;
    }

    outframes = -1;
    return 0;
}

void throughput_add_output(int bytes, int messages) {
    if (outframes < 0) {
        outframes = 1;
    } else {
        outframes++;
    }
    outbytes += bytes;
    outmessages += messages;
}

