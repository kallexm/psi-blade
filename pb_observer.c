#include "pb_observer.h"

#include <string.h>

pb_observer_retval_t pb_observer_init(pb_observer_t* observer)
{
    memset(observer->callbacks, 0, sizeof(pb_observer_cb_t));
    memset(observer->context,   0, sizeof(void*));
    return PB_OBSERVER_SUCCESS;
}

pb_observer_retval_t pb_observer_notify(pb_observer_t* observer)
{
    for (int i = 0; i < observer->subscribers_max_num; i++)
    {
        if (observer->callbacks[i] != NULL)
        {
            (observer->callbacks[i])(observer->context[i]);
        }
    }
    return PB_OBSERVER_SUCCESS;
}

pb_observer_retval_t pb_observer_subscribe(pb_observer_t* observer, pb_observer_cb_t cb, void* context)
{
    if (cb == NULL)
    {
        return PB_OBSERVER_FAILURE;
    }

    for (int i = 0; i < observer->subscribers_max_num; i++)
    {
        if (observer->callbacks[i] == NULL)
        {
            observer->callbacks[i] = cb;
            observer->context[i]   = context;
            return PB_OBSERVER_SUCCESS;
        }
    }
    return PB_OBSERVER_FAILURE;
}

pb_observer_retval_t pb_observer_unsubscribe(pb_observer_t* observer, pb_observer_cb_t cb)
{
    for (int i = 0; i < observer->subscribers_max_num; i++)
    {
        if (observer->callbacks[i] != cb)
        {
            observer->callbacks[i] = NULL;
            observer->context[i]   = NULL;
            return PB_OBSERVER_SUCCESS;
        }
    }
    return PB_OBSERVER_FAILURE;
}
