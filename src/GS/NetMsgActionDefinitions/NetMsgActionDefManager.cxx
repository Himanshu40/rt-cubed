/*      N E T M S G A C T I O N D E F M A N A G E R . C X X
 * BRL-CAD
 *
 * Copyright (c) 2009 United States Government as represented by
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
/** @file NetMsgActionDefManager.cxx
 *
 * Brief description
 *
 */

#include "GS/NetMsgActionDefinitions/NetMsgActionDefManager.h"


NetMsgActionDefManager::NetMsgActionDefManager()
{
}

NetMsgActionDefManager::~NetMsgActionDefManager()
{
}

void NetMsgActionDefManager::registerNetMsgActionDef(
		AbstractNetMsgActionDef& def)
{
}

unsigned int NetMsgActionDefManager::unregisterNetMsgActionDef(
		AbstractNetMsgActionDef& def)
{
	return 0;
}

AbstractNetMsgActionDef& NetMsgActionDefManager::unregisterNetMsgActionDef(
		unsigned int msgType)
{
}

AbstractNetMsgActionDef& NetMsgActionDefManager::getNetMsgActionDef(unsigned int msgType)
{
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
