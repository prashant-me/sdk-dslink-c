#if !defined( __SAMPLELINK_H__ )
#define __SAMPLELINK_H__
    #include "dslink/dslink.h"

#if defined( __cplusplus )
    extern "C" {
#endif // #if defined( __cplusplus )

    /// DSLink의 callbak 함수이다.
    void init( DSLink * );

    /// DSLink의 callback 함수이다.
    void connected( DSLink * );

    /// DSLink의 callback 함수이다.
    void disconnected( DSLink * );

#if defined( __cplusplus )
    }
#endif // #if defined( __cplusplus )

#endif // endif 