#define LOG_TAG "sampleLink"

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <dslink/dslink.h>
#include <dslink/log.h>
#include <dslink/ws.h>

#include "sampleLink.h"
#include "sampleQueue.h"

static DSNode *g_node = NULL;
static DSLink *g_link = NULL;

// 노드 10000개
static const  uint16_t NODECNT  = 10000;

// 10000개의 노드 생성 명령을 Enqueue 하는 Thread.
static void *ThreadFunc_create( void *arg ); 

// 노드 생성 명령을 Dequeue 해서 처리하는 uv_timer 함수
static void ConsumerCreateProc( uv_timer_t *handle );

static void SendDefaultResponse( json_t *rid, json_t *updates );

static void InvokeCreate( DSLink *link, DSNode *node,json_t *rid, json_t *params, ref_t *stream_ref);

// DSLink의 callback 함수이다.
// 테스트를 위한 기본 노드를 만든다. 'TEST BUTTON' 이름을 갖는 Invoke node 이며
// invoke 시 위의 Thread를 실행한다.
void init( DSLink * link )
{
    DSNode *newNode = NULL;
    json_t *columns = NULL;
    json_t *row     = NULL;

    //dslink_log_set_lvl("debug");
    log_info("init\n");
    
    g_link  = link;
    g_node  = link->responder->super_root;

    // { 10000개 노드 생성테스트를 위한 Invokable Node 생성 
    newNode = dslink_node_create( g_node, "btnCreate", "node");
    if( newNode == NULL ) {
        log_fatal("Failed to create a node\n");
        return;
    }

    newNode->on_invocation = InvokeCreate;
    dslink_node_set_meta_new( g_link, newNode, "$name", json_string_nocheck("Create") );
    dslink_node_set_meta_new( g_link, newNode, "$invokable", json_string_nocheck("read") );
    
    columns = json_array();
    row     = json_object();
    json_object_set_new( row, "name", json_string_nocheck("message"));
    json_object_set_new( row, "type", json_string_nocheck("string"));
    json_array_append_new( columns, row );

    dslink_node_set_meta_new( g_link, newNode, "$columns", columns );

    if( dslink_node_add_child( g_link, newNode )  != 0 ) {
        log_fatal("Failed to add a new node\n");
        dslink_node_tree_free(g_link, newNode);
    }
    // }


    return;
}

/// DSLink의 callback 함수이다.
void connected( DSLink *link )
{
    (void) link;
    log_info("connected\n");
}

/// DSLink의 callback 함수이다.
void disconnected( DSLink *link)
{
    (void) link;
    log_info("disconnected\n");
}

// Create 버튼으로 invoked 함수
void InvokeCreate(DSLink *link, DSNode *node,json_t *rid, json_t *params, ref_t *stream_ref)
{
    (void) link;
    (void) node;
    (void) rid;
    (void) params;
    (void) stream_ref;

    pthread_t   thread;
    json_t      *updates     = NULL;
    int32_t     rst          = 0;
    char        msg[255 + 1] = {0,};

    static SampleQueueCtx *queueCtx = NULL;
    static uv_timer_t     *uvTimer  = NULL;

    log_info("Invoke Create\n");

    do {
        if( queueCtx == NULL ) {
            queueCtx = NewSampleQueueCtx();
            SetSampleQueueMaxCnt( queueCtx, (uint32_t) NODECNT );
        }

        if( uvTimer == NULL ) {
            uvTimer         = (uv_timer_t *) dslink_calloc( 1, sizeof( uv_timer_t ) );
            uvTimer->data   = (void *)queueCtx;
            uv_timer_init( &g_link->loop, uvTimer );
        }

        // uv timer에 ConsumerCreateProc 함수가 20ms 주기로 호출되도록 한다.
        uv_timer_start( uvTimer, ConsumerCreateProc, 0, 20 ); 

        // { CREATE THREAD
        // 노드를 생성하는 Thread를 실행한다. 
        rst = pthread_create( &thread, NULL, ThreadFunc_create, (void *)queueCtx );
        if( rst != 0 ) {
            snprintf( msg, sizeof(msg) - 1 , "Failed to create a creator thread : %d", rst );
            break;
        }

        snprintf( msg, sizeof(msg) - 1 , "Succeed to create a creator thread");
    } while( 0 ); 

    //{ SEND RESPONSE
    log_info("Send response : %s\n", msg );

    updates = json_array();
    json_array_append_new( updates, json_string(msg) );
    SendDefaultResponse( rid, updates );
    json_decref(updates);
    //}

    return;
}



/**
* Invoke에 대한 기본 응답을 보낸다.  
*
* @param json_t *rid 응답 아이디
* @param json_t *updates 응답 Data 배열
*/
static void SendDefaultResponse(json_t *rid, json_t *updates)
{
    assert( rid     != NULL && "rid is null"        );
    assert( updates != NULL && "updates is null"    );

    json_t *top      = NULL;
    json_t *_updates = NULL;
    json_t *resps    = NULL;
    json_t *resp     = NULL;

    do {
        top = json_object();
        if( top == NULL )
            break;

        resps = json_array();
        if( resps == NULL )
            break;
        json_object_set_new_nocheck(top, "responses", resps);

        resp = json_object();
        if( NULL == resp )
            break;

        _updates = json_array();
        if( NULL == _updates)
            break;
        json_array_append(_updates, updates);

        json_object_set_new_nocheck(resp, "updates", _updates);
        _updates = NULL;

        json_object_set_new_nocheck(resp, "stream", json_string("closed"));
        json_object_set_nocheck(resp, "rid", rid);
        
        json_array_append_new( resps, resp );
        resp = NULL;

        LOG_LVL_CHK( LOG_LVL_DEBUG ) {
            log_debug("Response msg %s\n", json_dumps( top, JSON_PRESERVE_ORDER));
        }
    
        dslink_ws_send_obj(g_link, top );
    } while( 0 );

    if( _updates != NULL )
        json_decref( _updates );

    if( resp != NULL )
        json_decref( resp );

    if( resps != NULL )
        json_decref( resps );

    if( top != NULL )
        json_decref( top );
}

/**
 */
void *ThreadFunc_create( void *arg )
{
    (void) arg;
    log_info( "Start the creator thread\n");

    char            nodeName[255 + 1]   = {0,};
    uint32_t        idx                 = 0; 
    SampleQueueCtx  *queueCtx           = (SampleQueueCtx *) arg;
    SampleQueueData *data               = NULL;
    
    do {
        if( queueCtx == NULL ) 
            break;

        log_info( "start to enqueue a command for to create node\n");

        // 시간 측정 시작을 알리는 명령
        data = NewSampleQueueData( DLCMD_SET_TIME ); 
        SampleEnqueue( queueCtx, data );

        // nodes 이름의 부모노드 생성 명령
        data = NewSampleQueueData( DLCMD_CREATE_NODE );
        SetSampleQueueData( data, NULL, "nodes", NULL );
        SampleEnqueue( queueCtx, data );

        for( idx = 0 ; idx < NODECNT ; idx++ ) {
            snprintf(nodeName, sizeof( nodeName ) - 1, "testnode_%05d", (idx + 1) ); 
            
            data = NewSampleQueueData( DLCMD_CREATE_NODE );
            SetSampleQueueData( data, "nodes", (const char *) nodeName, NULL );
            SampleEnqueue( queueCtx, data );

            if( (idx+1) % 1000 == 0 ) {
                log_info("%d creation commands enqueued.\n", idx + 1);
            }
        }

        // 시간 측정 종료를 알리는 Data
        data = NewSampleQueueData( DLCMD_END_TIME );
        SampleEnqueue( queueCtx, data );

        log_info( "end to enqueue a command for to create node\n");
    } while ( 0 );


    log_info( "End the creator thread\n");

    return NULL;
}


static void ConsumerCreateProc( uv_timer_t *handle )
{
    (void) handle;

    static 
    struct timeval  bgnTime         = {0,0};
    static
    struct timeval  endTime         = {0,0};

    static uint8_t  receivedEndTime = 0;
    static uint8_t  receivedBgnTime = 0;
    
    SampleQueueCtx  *queueCtx       = (SampleQueueCtx *) handle->data;
    SampleQueueData *data           = NULL;

    DSNode          *parentNode     = g_node;
    DSNode          *newNode        = NULL;
    ref_t           *refParent      = NULL;

    //log_debug("Node creation command uv timer function invoked\n");
    if( queueCtx == NULL ) {
        uv_timer_stop( handle );
        return;
    }

    size_t count = 100;
    do {
        data = (SampleQueueData *)SampleDequeue( queueCtx );
        if(data == NULL ) {
            log_warn("Node creation queue emptied or failed to lock the mutex.\n");
            break;
        }

        switch( data->cmd ) {
            case DLCMD_SET_TIME:
                gettimeofday( &bgnTime, NULL );
                receivedBgnTime = (uint8_t)1;  
                break;
            case DLCMD_END_TIME:
                gettimeofday( &endTime, NULL );
                receivedEndTime = (uint8_t)1;
                break;
            case DLCMD_CREATE_NODE: 
                if( data->parentName != NULL ) {
                    // 부모노드가 필요하다면 만든다.
                    if( g_node->children == NULL 
                        || ( g_node->children != NULL && (refParent = dslink_map_get( g_node->children, (void*) data->parentName )) == NULL) ) {
                        parentNode = dslink_node_create( g_node, data->parentName, "node");
                        dslink_node_set_meta_new( g_link, parentNode, "$name", json_string( data->parentName ) );

                        if( dslink_node_add_child( g_link, parentNode ) != 0 ) {
                            dslink_node_tree_free(g_link, parentNode);
                            log_fatal("failed to create a '%s' parent node\n", data->parentName );
                            parentNode = NULL;
                            break;
                        }
                    }

                    if( refParent != NULL ) 
                        parentNode = (DSNode *) refParent->data;
                }

                newNode = dslink_node_create( parentNode, data->nodeName, "node");
                dslink_node_set_meta_new( g_link, newNode, "$name", json_string( data->nodeName ) );
                if( dslink_node_add_child( g_link, newNode ) != 0 ) {
                    dslink_node_tree_free(g_link, newNode);
                    log_fatal("failed to create a '%s' node\n", data->nodeName );
                }
                break;
            case DLCMD_SET_VALUE:
                log_warn("recived a unexpected msg.\n");
                break;
        }

        DeleteSampleQueueData( data );
    } while( count-- );

    if( receivedEndTime != 0 ) {
        int64_t diffTime = 0;
        if( receivedBgnTime != 0 )
            diffTime = (( endTime.tv_sec * 1000 ) + ( endTime.tv_usec / 1000.0)) - ( ( bgnTime.tv_sec * 1000 ) + ( bgnTime.tv_usec / 1000.0) );
        else
            diffTime = -1;

        newNode = NULL;
        if( g_node->children == NULL 
            || ( g_node->children != NULL && (refParent = dslink_map_get( g_node->children, (void*) "time" )) == NULL) ) {
            
            newNode = dslink_node_create( g_node, "time", "node");
            dslink_node_set_meta_new( g_link, newNode, "$name", json_string( "time(ms)" ) );
            dslink_node_set_meta_new( g_link, newNode, "$type", json_string_nocheck("number") );

            if( dslink_node_add_child( g_link, newNode ) != 0 ) {
                dslink_node_tree_free(g_link, newNode);
                log_fatal("failed to create a time node\n");
            }
        }

        if( newNode == NULL && refParent != NULL )
            newNode = (DSNode *)refParent->data;

        if( newNode != NULL )
            dslink_node_set_value( g_link, newNode, json_integer((json_int_t) diffTime) );

        log_info("performance test finished. total node creation time : %"PRId64"ms\n", diffTime );
        uv_timer_stop( handle );
        receivedBgnTime = 0;
        receivedEndTime = 0;
    }

    return;
}



