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
#include <map>

#include "common.h"
#include "Stigmergy.h"

#include "CmcAdapter.h"
#include "CmcContext.h"

#include "MessagePkt.h"

#include "../mycmc/MyCmc.h"

using std::cout;
using std::endl;

using marusa::swms::Stigmergy;
using marusa::swms::CmcAdapter;

using marusa::swms::bytecpy;

using marusa::swms::JOB_ID;
using marusa::swms::TASK_ID;
using marusa::swms::TASK_PKT_HEADER;

class MySGYListener : public Stigmergy::SGYCallbackListener
{
public:
	void onRecvTask(const Stigmergy::SGYContext &context,
					const BYTE *task)
	{
		std::cout << "MySGYListener::onRecvJobId - come task" << std::endl;

		TASK_PKT_HEADER *task_pkt_header = (TASK_PKT_HEADER *)task;

		std::pair<JOB_ID, HOST_ID> task_uid(task_pkt_header->job_id, task_pkt_header->task_id);
		mMapTasks[task_uid] = (BYTE *)malloc(sizeof(BYTE) * task_pkt_header->data_size);
		bytecpy((BYTE *)mMapTasks[task_uid], &task[sizeof(TASK_PKT_HEADER)], task_pkt_header->data_size);

		printf("\tJOB ID\t: %d\n", task_pkt_header->job_id);
		printf("\tTASK ID\t: %d\n", task_pkt_header->task_id);
		printf("\tDATA SIZE\t: %d\n", task_pkt_header->task_id);
	}

private:
	std::map<std::pair<JOB_ID, HOST_ID>, const BYTE *> mMapTasks;
	std::map<std::pair<JOB_ID, HOST_ID>, const BYTE *> mMapResults;

	int getTaskNum(){
		return ((this->mMapTasks).size());
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

