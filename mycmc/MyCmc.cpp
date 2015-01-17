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
#include <stdio.h>

#ifdef ___DEBUG_TRANS_TASK_IFA2SGY___
#include "MessagePkt.h"
using marusa::swms::MessagePkt;
#endif /* ___DEBUG_TRANS_TASK_IFA2SGY___ */

MyCmc::MyCmc(CmcAdapter::CmcCallbackListener *listener) : CmcAdapter(listener)
{
	myTCPListener = new MyTCPListener(listener);
}

HOST_ID MyCmc::connToStigmergy()
{
	if (this->mCl == nullptr){
		std::string ip_str;
		int port_no;

		getStyPos(ip_str, port_no);
		this->mCl = new TCPClient(inet_addr(ip_str.c_str()), port_no);
		mCl->set_on_reply_recv_listener(myTCPListener);
	}

	(this->mCl)->est_conn();
	HOST_ID hid = (this->mCl)->get_socket();

	return (hid);
}

int MyCmc::startListen()
{
	if (this->mSv == nullptr){
		int port_no;

		getPort(port_no);
		this->mSv = new TCPServer(port_no);
		mSv->set_on_reply_recv_listener(myTCPListener);
	}

	(this->mSv)->start_listening();

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

void bytecpy(BYTE *to,
			 const BYTE *from,
			 unsigned int len)
{
	while (len--){
		*to++ = *from++;
	}
}

int MyCmc::sendMessage(const HOST_ID &host_id,
					   const BYTE *msg,
					   const unsigned int &size_msg)
{
	BYTE *tmp = (BYTE *)malloc(sizeof(BYTE) * size_msg);
	unsigned int size_msg_tmp = size_msg;
	bytecpy(tmp, msg, size_msg);

	if (this->mCl != nullptr){
#ifdef ___DEBUG_TRANS_TASK_IFA2SGY___
	std::cout << "MyCmc::sendMessage - send_msg of mCl will called" << std::endl;
	printf("\tmsg type : %d\n", *tmp);
	printf("\tmsg size : %d\n", *((int *)&tmp[MessagePkt::SIZE_MSG_TYPE]));
	printf("\tmsg dmp : %s\n", &tmp[MessagePkt::SIZE_MSG_TYPE + MessagePkt::SIZE_DATA_SIZE]);
#endif /* ___DEBUG_TRANS_TASK_IFA2SGY___ */

		(this->mCl)->send_msg((MESSAGE *)tmp, size_msg_tmp);
	}
	else {
		marusalib::tcp::utilities::send_msg(host_id, (MESSAGE *)tmp, size_msg_tmp);
	}

	return (0);
}


MyCmc::MyTCPListener::MyTCPListener(CmcAdapter::CmcCallbackListener *listener)
{
	this->mCmcCallbackListener = listener;
}

void MyCmc::MyTCPListener::onRecv(RecvContext *context, MESSAGE *msg)
{
#ifdef ___DEBUG_TRANS_TASK_IFA2SGY___
	std::cout << "MyCmc::MyTCPListener::onRecv - recieved message" << std::endl;
#endif /* ___DEBUG_TRANS_TASK_IFA2SGY___ */
}
