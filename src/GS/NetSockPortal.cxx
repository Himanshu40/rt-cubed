/*               N E T S O C K P O R T A L . C X X
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
/** @file NetSockPortal.cxx
 *
 * Brief description
 *
 */

#include "GS/NetSockPortal.h"
#include "iBME/iBMECommon.h"
#include "GS/netMsg/NetMsg.h"
#include "GS/netMsg/RemHostNameSetMsg.h"

NetSockPortal::NetSockPortal(QObject* parent) :
	QTcpSocket(parent)
{
	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(
			moveDataFromSocketBuffer()));

}

NetSockPortal::~NetSockPortal()
{

}

void NetSockPortal::moveDataFromSocketBuffer()
{
	//get data off socket
	QByteArray data = this->readAll();

	//put into factory buffer & attempt a msg build.
	this->factory->addData(data);
	this->factory->attemptToMakeMsgs();

	if (this->hasMsg())
	{

		//Route the message according to the port status.
		if (this->portStatus == NetSockPortal::Ready)
		{
			//Normally, just emit a signal.
			emit msgReady();
		}
		else if (this->portStatus == NetSockPortal::NotConnected
				|| this->portStatus == NetSockPortal::Handshaking)
		{
			this->portStatus = NetSockPortal::Handshaking;
			NetMsg* msg = this->getNextMsg();

			if (msg->getMsgType() != REMHOSTNAMESET)
			{
				this->disconnect(PORTAL_HANDSHAKE_FAILURE);
				return;
			}

			RemHostNameSetMsg* rhnsm = (RemHostNameSetMsg*) msg;

			QString remoteHostname = rhnsm->getRemoteHostName();

			//TODO add a way to check for duplicate hosts

			if (remoteHostname.isEmpty())
			{
				this->disconnect(PORTAL_HANDSHAKE_FAILURE);
				return;
			}

			this->remHostName = remoteHostname;
			this->portStatus = NetSockPortal::Ready;
			emit portalHandshakeComplete();
		}
		else
		{
			//Should be failed
			this->disconnect(PORTAL_HANDSHAKE_FAILURE);
			return;
		}
	}
}

/**
 * This function blocks until all data is sent.
 */
void NetSockPortal::send(NetMsg& msg)
{
	QByteArray ba;

	msg.serialize(&ba);

	quint64 totalToSend = ba.size();
	quint64 thisSend = 0;
	quint64 totalSent = 0;

	//TODO needs to be a better way to do this
	while (totalSent < totalToSend)
	{
		thisSend = this->write(ba);

		//Check for socket failure
		if (thisSend == -1)
		{
			this->disconnect(PORTAL_WRITE_FAILURE);
			return;
		}

		//Check for Zero bytes sent
		if (thisSend == 0)
		{
			if (totalSent != totalToSend)
			{
				std::cerr << "Did not send all data from Msg to host: "
						<< this->remHostName.toStdString() << "\n";
			}
			return;
		}
		totalSent += thisSend;
	}

}

void NetSockPortal::disconnect(quint8 reason)
{
	//TODO finish Implementing this.
}

bool NetSockPortal::hasMsg()
{
	return this->factory->hasMsgsAvailable();
}

NetMsg* NetSockPortal::getNextMsg()
{
	return this->factory->getNextMsg();
}


QString NetSockPortal::getRemoteHostName()
{
	return this->remHostName;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
