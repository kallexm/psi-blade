#include "pb_sercom_handlers.h"

typedef struct
{
    pb_sercom_handler_t  app_int_handler;
    void*                context;
} pb_sercom_app_handler_t;

pb_sercom_app_handler_t app_handler[SERCOM_INST_NUM] = {0};


pb_sercom_handlers_retval_t pb_sercom_handlers_register_callback(Sercom* SERCOMn, pb_sercom_handler_t cb, void* context)
{   
    uint32_t sercom_number = SERCOM_INST_NUM;
    switch(SERCOMn)
    {
        case SERCOM0:
            sercom_number = 0;
            break;

        case SERCOM1:
            sercom_number = 1;
            break;

        case SERCOM2:
            sercom_number = 2;
            break;

        case SERCOM3:
            sercom_number = 3;
            break;

        case SERCOM4:
            sercom_number = 4;
            break;

        case SERCOM5:
            sercom_number = 5;
            break;

        default:
            return PB_SERCOM_HANDLERS_FAILURE;
    }

    if (app_handler[sercom_number].app_int_handler == NULL)
    {
        app_handler[sercom_number].app_int_handler = cb;
        app_handler[sercom_number].context         = context;
    }
    else
    {
        return PB_SERCOM_HANDLERS_IN_USE;
    }

    return PB_SERCOM_HANDLERS_SUCCESS;
}

pb_sercom_handlers_retval_t pb_sercom_handlers_unregister_callback(Sercom* SERCOMn)
{
    uint32_t sercom_number = SERCOM_INST_NUM;
    switch(SERCOMn)
    {
        case SERCOM0:
            sercom_number = 0;
            break;

        case SERCOM1:
            sercom_number = 1;
            break;

        case SERCOM2:
            sercom_number = 2;
            break;

        case SERCOM3:
            sercom_number = 3;
            break;

        case SERCOM4:
            sercom_number = 4;
            break;

        case SERCOM5:
            sercom_number = 5;
            break;

        default:
            return PB_SERCOM_HANDLERS_FAILURE;
    }

    app_handler[sercom_number].app_int_handler = NULL;
    app_handler[sercom_number].context         = NULL;

    return PB_SERCOM_HANDLERS_SUCCESS;
}


void SERCOM0_Handler(void)
{   
    uint32_t sercom_number = 0;
    if (app_handler[sercom_number].app_int_handler == NULL)
    {
        pb_critical_section_enter();
        app_handler[sercom_number].app_int_handler(app_handler[sercom_number].context);
        pb_critical_section_leave();
    }
}

void SERCOM1_Handler(void)
{
    uint32_t sercom_number = 1;
    if (app_handler[sercom_number].app_int_handler == NULL)
    {
        pb_critical_section_enter();
        app_handler[sercom_number].app_int_handler(app_handler[sercom_number].context);
        pb_critical_section_leave();
    }
}

void SERCOM2_Handler(void)
{
    uint32_t sercom_number = 2;
    if (app_handler[sercom_number].app_int_handler == NULL)
    {
        pb_critical_section_enter();
        app_handler[sercom_number].app_int_handler(app_handler[sercom_number].context);
        pb_critical_section_leave();
    }
}

void SERCOM3_Handler(void)
{
    uint32_t sercom_number = 3;
    if (app_handler[sercom_number].app_int_handler == NULL)
    {
        pb_critical_section_enter();
        app_handler[sercom_number].app_int_handler(app_handler[sercom_number].context);
        pb_critical_section_leave();
    }
}

void SERCOM4_Handler(void)
{
    uint32_t sercom_number = 4;
    if (app_handler[sercom_number].app_int_handler == NULL)
    {
        pb_critical_section_enter();
        app_handler[sercom_number].app_int_handler(app_handler[sercom_number].context);
        pb_critical_section_leave();
    }
}

void SERCOM5_Handler(void)
{
    uint32_t sercom_number = 5;
    if (app_handler[sercom_number].app_int_handler == NULL)
    {
        pb_critical_section_enter();
        app_handler[sercom_number].app_int_handler(app_handler[sercom_number].context);
        pb_critical_section_leave();
    }
}
