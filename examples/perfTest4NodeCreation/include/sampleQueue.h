/**
 * 쓰레드간 data 전달을 위한 queue
 */

#if !defined( __SAMPLEQUEUE_H__ )
#define __SAMPLEQUEUE_H__ 

#include <stdint.h>
#include <pthread.h>


typedef struct SampleQueueObj {
    struct SampleQueueObj *next;
    struct SampleQueueObj *prev;

    void        *data;

} SampleQueueObj;

typedef struct SampleQueueCtx {
    pthread_mutex_t mutex;

    uint32_t        maxCnt;
    uint32_t        cnt;
    SampleQueueObj  *head;
    SampleQueueObj  *tail;
} SampleQueueCtx;

typedef enum _SampleCommand_ {
    DLCMD_SET_TIME     = 0
    ,DLCMD_END_TIME    = 1
    ,DLCMD_CREATE_NODE = 2
    ,DLCMD_SET_VALUE   = 3
} ESampleCmd;

typedef struct SampleQueueData {
    ESampleCmd cmd;
    char       *parentName;
    char       *nodeName;
    char       *value;
} SampleQueueData ;

SampleQueueCtx * NewSampleQueueCtx    ( );
void    SetSampleQueueMaxCnt        ( SampleQueueCtx *ctx, uint32_t maxCnt );
int32_t SampleEnqueue               ( SampleQueueCtx *ctx, void *data );
void    *SampleDequeue              ( SampleQueueCtx *ctx );

SampleQueueData *NewSampleQueueData ( ESampleCmd cmd );
void DeleteSampleQueueData          ( void *data );
void SetSampleQueueData             ( SampleQueueData *obj, const char *parentName, const char *nodeName, const char *value );

#endif // __SAMPLEQUEUE_H__
