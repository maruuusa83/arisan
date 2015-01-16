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
#include "./MyCmc.h"

MyCmc::MyCmc(CmcAdapter::CmcCallbackListener *listener) : CmcAdapter(listener)
{

}

HOST_ID MyCmc::connToStigmergy()
{
	cout << "in MyCmc::connToStigmergy" << endl;

	if (this->mCl == nullptr){
		std::string ip_str;
		int port_no;

		getStyPos(ip_str, port_no);
		this->mCl = new TCPClient(inet_addr(ip_str.c_str()), port_no);
	}

	(this->mCl)->est_conn();
	HOST_ID hid = (this->mCl)->get_socket();

	cout << "out MyCmc::connToStigmergy" << endl;
	return (hid);
}

int MyCmc::startListen()
{
	cout << "in MyCmc::startListen" << endl;

	if (this->mSv == nullptr){
		int port_no;

		getPort(port_no);
		this->mSv = new TCPServer(port_no);
	}

	(this->mSv)->start_listening();

	cout << "out MyCmc::startListen" << endl;
	return (0);
}

int MyCmc::getStyPos(std::string &ip, int &port)
{
	ip = "127.0.0.1";
	getPort(port);

	return (0);
}

int MyCmc::getPort(int &port)
{
	port = 1234;

	return (0);
}
