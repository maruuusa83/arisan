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

#include "../tcp/TCPClient.h"
#include "../tcp/TCPServer.h"

using std::cout;
using std::endl;

using marusa::swms::CmcAdapter;
using marusa::swms::HOST_ID;

using marusalib::tcp::TCPClient;
using marusalib::tcp::TCPServer;

class MyCmc : public CmcAdapter
{
public:
	MyCmc(CmcAdapter::CmcCallbackListener *listener);

	HOST_ID connToStigmergy();

	int startListen();

private:
	TCPClient *mCl = nullptr;
	TCPServer *mSv = nullptr;

	int getStyPos(std::string &ip, int &port);
	int getPort(int &port);
};

