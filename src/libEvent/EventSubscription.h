/*             E V E N T S U B S C R I P T I O N . H
 * BRLCAD
 *
 * Copyright (c) 2010 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file EventSubscription.h
 *
 * Brief description
 *
 */

#ifndef __EVENTSUBSCRIPTION_H__
#define __EVENTSUBSCRIPTION_H__

#define ALL_TYPES	0
#define ALL_PUBLISHERS	0

#include "EventSubscriber.h"
#include "EventPublisher.h"

class EventSubscription
{
public:
    EventSubscription(EventSubscriber* sub, quint32 eventType = ALL_TYPES,
	    EventPublisher* pub = ALL_PUBLISHERS);
    virtual ~EventSubscription();

    EventPublisher* getPublisher();
    quint32 getEventType();
    EventSubscriber* getEventSubscriber();

private:
    EventPublisher* _pub;
    quint32 _eventType;
    EventSubscriber* _sub;
};

#endif /* __EVENTSUBSCRIPTION_H__ */

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
