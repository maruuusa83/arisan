/********************************************************************************
 * Copyright (C) 2014 Daichi Teruya (@maruuusa83)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is destributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *******************************************************************************/
#include <iostream>
#include <string>

#include "common.h"
#include "CmcAdapter.h"
#include "Stigmergy.h"
#include "CmcContext.h"

#include "../tcp/TCPClient.h"
#include "../tcp/TCPServer.h"
#include "../tcp/TCPSettings.h"
#include "../tcp/TCPUtilities.h"

using std::cout;
using std::endl;

using marusa::swms::CmcAdapter;
using marusa::swms::HOST_ID;
using marusa::swms::BYTE;
using marusa::swms::Stigmergy;

using marusalib::tcp::MESSAGE;
using marusalib::tcp::TCPClient;
using marusalib::tcp::TCPServer;
using marusalib::tcp::OnReplyRecvListener;
using marusalib::tcp::RecvContext;


class MyCmc : public CmcAdapter
{
public:
	MyCmc(CmcAdapter::CmcCallbackListener *listener);
	MyCmc(CmcAdapter::CmcContext *context, CmcAdapter::CmcCallbackListener *listener);

	HOST_ID connToStigmergy();

	int startListen();

private:
	class MyTCPListener;

	TCPClient *mCl = nullptr;
	TCPServer *mSv = nullptr;
	MyTCPListener *myTCPListener = nullptr;

	int getStyPos(std::string &ip, int &port);
	int getPort(int &port);

	int sendMessage(const HOST_ID &host_id,
					const BYTE *msg,
					const unsigned int &size_msg);
};

class MyCmc::MyTCPListener : public OnReplyRecvListener
{
public:
	MyTCPListener(CmcAdapter::CmcCallbackListener *listener);
	MyTCPListener(CmcAdapter::CmcContext *context, CmcAdapter::CmcCallbackListener *listener);

	void onRecv(RecvContext *context, MESSAGE *msg);

	int setSGYContext(Stigmergy::SGYContext *context);

private:
	CmcAdapter::CmcCallbackListener *mCmcCallbackListener = nullptr;
	CmcAdapter::CmcContext *mCmcContext = nullptr;
};


