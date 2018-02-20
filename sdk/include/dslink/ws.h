#ifndef SDK_DSLINK_C_WS_H
#define SDK_DSLINK_C_WS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mbedtls/ecdh.h>
#include <jansson.h>

#include "dslink/dslink.h"
#include "dslink/socket.h"
#include "dslink/err.h"
#include "dslink/url.h"

int dslink_handshake_connect_ws(Url *url,
                                mbedtls_ecdh_context *key,
                                const char *uri,
                                const char *tempKey,
                                const char *salt,
                                const char *dsId,
                                const char *token,
                                Socket **sock);
void dslink_handshake_handle_ws(DSLink *link, link_callback on_requester_ready_cb);

int dslink_ws_send_obj(DSLink* link, json_t* obj);

void process_send_events(uv_prepare_t* handle);

#ifdef __cplusplus
}
#endif

#endif // SDK_DSLINK_C_WS_H
