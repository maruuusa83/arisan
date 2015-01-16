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

#include "common.h"
#include "CmcAdapter.h"

using std::cout;
using std::endl;

using marusa::swms::CmcAdapter;
using marusa::swms::JOB_ID;
using marusa::swms::HOST_ID;

class MyCMC : public CmcAdapter
{
public:
	MyCMC(CmcAdapter::CmcCallbackListener *listener) : CmcAdapter(listener)
	{

	}

	HOST_ID connToStigmergy()
	{
		cout << "in MyCMC::connToStigmergy" << endl;

		cout << "out MyCMC::connToStigmergy" << endl;
		return (0);
	}

	int startListen()
	{
		cout << "in MyCMC::startListen" << endl;

		return (0);
	}
};

