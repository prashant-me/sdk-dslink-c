/**
 * 쓰레드간 data 전달을 위한 queue
 */

#define LOG_TAG "sampleQueue"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include <pthread.h>

#include <dslink/dslink.h>
#include <dslink/log.h>
#include <dslink/utils.h>

#include "sampleQueue.h"

/**
 * 새로운 SampleQueueCtx Context를 반환한다.
 * 
 * @return SampleQueueCtx *, NULL 실패
 */
SampleQueueCtx *NewSampleQueueCtx( )
{
    SampleQueueCtx *ctx = NULL;

    ctx = (SampleQueueCtx *)dslink_calloc( 1, sizeof( SampleQueueCtx ) );
    pthread_mutex_init  ( &ctx->mutex, NULL );

    ctx->maxCnt = 0;
    ctx->cnt    = 0;

    return ctx;
}

void SetSampleQueueMaxCnt( SampleQueueCtx *ctx, uint32_t maxCnt ) 
{
    ctx->maxCnt = maxCnt;
}

/**
 * Enqueue 한다.
 * 
 * @param SampleQueueCtx *  : Queue를 관리하기 위한 Context
 * @param void *            : Enqueue 대상 데이타
 * @return int32_t          : 성공시 Enqueue된 개수, -1 실패
 * @warning 함수가 성공했을때, data를 외부에서 해제하지 않는다.
 */
int32_t SampleEnqueue ( SampleQueueCtx *ctx, void *data ) 
{
    assert( ctx != NULL && "SampleQueueCtx is null");

    int32_t rst = 0;
    int32_t cnt = 0;

    SampleQueueObj *queueObj = NULL;

    do {
        queueObj = (SampleQueueObj *) dslink_calloc( 1, sizeof(SampleQueueObj) );
        if( queueObj == NULL ) {
            log_fatal( "Failed to allocate memory");
            rst = -1;
            break;
        }

        queueObj->next = NULL;
        queueObj->prev = NULL;
        queueObj->data = data;

        rst = pthread_mutex_lock( &ctx->mutex );
        if( rst != 0 ) {
            log_fatal("Failed to lock\n");
            rst = -1;
            break;
        }

        if( ctx->head == NULL ) {
            ctx->head = queueObj;
        } else {
            ctx->head->prev = queueObj;
            queueObj->next  = ctx->head;
            ctx->head       = queueObj;
        }

        if( ctx->tail == NULL ) 
            ctx->tail = queueObj;
        
        queueObj  = NULL;
        cnt       = ctx->cnt;
    } while( 0 );

    if( rst == 0 )
        pthread_mutex_unlock( &ctx->mutex);

    if( rst != 0 && queueObj != NULL )
        dslink_free( queueObj );

    if( rst == 0 )
        rst = cnt;

    return rst;
}

/**
 * Dequeue 한다.
 * 
 * @param SampleQueueCtx* : Queue를 관리하기 위한 Context
 * @param unsigned int : lock timeout 시간 단위 millisecond
 * @return void * : data, NULL 타임아웃 또는 Empty
 */
void *SampleDequeue( SampleQueueCtx *ctx )
{
    assert( ctx != NULL && "SampleQueueCtx is null.");


    SampleQueueObj *queueObj = NULL;
    void            *data    = NULL;
    int             lockRst  = 0;

    do {
        lockRst = pthread_mutex_lock( &ctx->mutex );
        if( lockRst != 0 ) {
            log_fatal( "Failed to lock")
            break;
        }
        
        if( ctx->tail == NULL )
            break;
        queueObj = ctx->tail;

        ctx->tail = ctx->tail->prev;
		if( ctx->tail != NULL )
			ctx->tail->next = NULL;

        if( ctx->tail == NULL )
            ctx->head = NULL;

        ctx->cnt -= 1;
    } while( 0 );

    if( lockRst == 0 )
        pthread_mutex_unlock( &ctx->mutex );

    if( queueObj != NULL ) {
        data = queueObj->data;
        dslink_free( queueObj );
    }

    return data;
}

SampleQueueData *NewSampleQueueData ( ESampleCmd cmd )
{
    SampleQueueData *rst = NULL;

    rst = (SampleQueueData *)dslink_calloc(1, sizeof(SampleQueueData) );
    rst->cmd = cmd;

    return rst;
}

void DeleteSampleQueueData( void *data )
{
    SampleQueueData *obj = (SampleQueueData *)data;
    if( obj == NULL )
        return;

    if( obj->parentName != NULL )
        dslink_free( obj->parentName );

    if( obj->nodeName != NULL )
        dslink_free(obj->nodeName);

    if( obj->value != NULL )
        dslink_free( obj->value );

    dslink_free( obj );
}

void SetSampleQueueData( SampleQueueData *obj, const char *parentName, const char *nodeName, const char *value )
{
    if( obj == NULL )
        return;

    if( parentName != NULL ) 
        obj->parentName = dslink_strdup( parentName );

    if( nodeName != NULL )
        obj->nodeName = dslink_strdup( nodeName );

    if( value != NULL )
        obj->value  = dslink_strdup( value );
}
