#ifndef PB_SERCOM_HANDLERS_H__
#define PB_SERCOM_HANDLERS_H__

#include "samd21.h"

typedef void (*pb_sercom_handler_t)(void* context);

typedef enum
{
    PB_SERCOM_HANDLERS_SUCCESS,
    PB_SERCOM_HANDLERS_FAILURE,
    PB_SERCOM_HANDLERS_IN_USE,
}pb_sercom_handlers_retval_t;


pb_sercom_handlers_retval_t pb_sercom_handlers_register_callback(Sercom* SERCOMn, pb_sercom_handler_t cb, void* context);
pb_sercom_handlers_retval_t pb_sercom_handlers_unregister_callback(Sercom* SERCOMn);

#endif /* PB_SERCOM_HANDLERS_H__ */