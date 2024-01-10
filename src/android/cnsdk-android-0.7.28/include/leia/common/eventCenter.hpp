/*
 * Copyright 2023 (c) Leia Inc.  All rights reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Leia Inc. and its suppliers, if any.  The
 * intellectual and technical concepts contained herein are
 * proprietary to Leia Inc. and its suppliers and may be covered
 * by U.S. and Foreign Patents, patents in process, and are
 * protected by trade secret or copyright law.  Dissemination of
 * this information or reproduction of this materials strictly
 * forbidden unless prior written permission is obtained from
 * Leia Inc.
 */
#ifndef CNSDK_LEIA_COMMON_EVENT_CENTER_HPP
#define CNSDK_LEIA_COMMON_EVENT_CENTER_HPP

#include "leia/common/eventCenter.h"

namespace leia {

using Event                 = leia_event;
using EventListenerCallback = leia_event_listener_callback;

class EventListener {
public:
    EventListener(EventListenerCallback callback, void* userData) : _listener(leia_event_listener_alloc())
    {
        leia_event_listener_set_callback(_listener, callback, userData);
    }
    ~EventListener()
    {
        Unregister();
        leia_event_listener_free(_listener);
    }
    void Register()
    {
        leia_event_center_add_listener(_listener);
    }
    void Unregister()
    {
        leia_event_center_remove_listener(_listener);
    }

private:
    leia_event_listener* _listener;
};

inline void DefaultEventHandler(Event const* event)
{
    leia_default_event_handler(event);
}

} // namespace leia

#endif // CNSDK_LEIA_COMMON_EVENT_CENTER_HPP
