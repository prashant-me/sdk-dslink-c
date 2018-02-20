/**
 * dslink_sdk_c library thread unsafe 로 인한 memory corruption을 방지하기 위해 
 * dslink_sdk_c api는 link->loop 내에서 사용돼야 한다. 
 * 그러므로 멀티쓰레드 환경에서 노드의 생성, 삭제, 추가, 변경과 같은 일련의 작업은 queue 를 통해 전달되어 
 * link->loop thread 내에서 실행돼야 하며, 본 프로그램은 수만건의 노드를 갖는 OPCUA 링크을 지원하기 위한 
 * 성능테스트를 위해 만들어졌다. 
 * 
 * 본 프로그램의 조건은 다음과 같다. 
 * 10000건의 노드를 생성, 링크의 브로커의 요청에 대한 응답성을 위해 link->loop 내에서 20ms 단위로 queue를 확인한다.
 * 10000건의 노드 생성 명령을 만드는 쓰레드는 생성명령 앞뒤에 시간 측정을 위한 명령을 포함한다. 
 * 10000건의 노드는 1 깊이의 부모노드를 갖는다. 
 * 
 * 이미 만들어진 노드를 삭제하는 기능을 지원하지 않으므로, 재테스트를 위해서는 재시작 해야한다.
 */
#include <stdio.h>

#include <dslink/dslink.h>

#include "sampleLink.h"

static const char *LINK_NAME = "SAMPLE2-LINK";

int main(int argc, char **argv)
{

    DSLinkCallbacks cbs = {
        init,           		/// init_cb
        connected,      		/// on_connected_cb
        disconnected,   	  /// on_disconnected_cb
        NULL            		/// on_requester_ready_cb
    };

    return dslink_init( argc, argv, LINK_NAME, 0, 1, &cbs );   
}