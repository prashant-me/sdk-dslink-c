#ifndef SDK_DSLINK_C_MESSAGE_UTILS_H
#define SDK_DSLINK_C_MESSAGE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jansson.h>

#include "dslink/col/vector.h"

json_t* merge_queue_messages(Vector* send_queue);

#ifdef __cplusplus
}
#endif

#endif // SDK_DSLINK_C_MESSAGE_UTILS_H
