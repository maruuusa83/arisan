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
#include "Stigmergy.h"

#include "CmcAdapter.h"
#include "CmcContext.h"

#include "../mycmc/MyCmc.h"

using std::cout;
using std::endl;

using marusa::swms::Stigmergy;
using marusa::swms::CmcAdapter;

class MySGYListener : public Stigmergy::SGYCallbackListener
{
public:
	void onRecvTask(const Stigmergy::SGYContext &context,
					const BYTE *task) const
	{
		std::cout << "MySGYListener::onRecvJobId - come task" << std::endl;
	}
};

int main()
{
	MySGYListener *listener = new MySGYListener();
	CmcAdapter::CmcCallbackListener *cmcCL = new CmcAdapter::CmcCallbackListener();

	CmcAdapter::CmcContext *cmcContext = new CmcAdapter::CmcContext();
	cmcContext->setSGYCallbackListener(listener);
	MyCmc *cmc = new MyCmc(cmcContext, cmcCL);

	Stigmergy sgy(cmc);
	sgy.startStigmergy();

	return (0);
}

