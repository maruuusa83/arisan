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
#include <fstream>
#include <string>
#include <map>
#include <fstream>

#include "common.h"
#include "Stigmergy.h"

#include "Result.h"

#include "CmcAdapter.h"
#include "CmcContext.h"

#include "MessagePkt.h"

#include "../mycmc/MyCmc.h"

using std::cout;
using std::endl;

using marusa::swms::Stigmergy;
using marusa::swms::CmcAdapter;
using marusa::swms::Result;

using marusa::swms::bytecpy;

using marusa::swms::JOB_ID;
using marusa::swms::TASK_ID;
using marusa::swms::TASK_PKT_HEADER;

std::map<int, int> worker_map;

class MySGYListener : public Stigmergy::SGYCallbackListener
{
public:
	void onRecvTask(const Stigmergy::SGYContext &context,
					const BYTE *task,
                    const HOST_ID hostid)
	{
		std::cout << "MySGYListener::onRecvJobId - come task" << std::endl;

		TASK_PKT_HEADER *task_pkt_header = (TASK_PKT_HEADER *)task;

		std::pair<JOB_ID, HOST_ID> task_uid(task_pkt_header->job_id, task_pkt_header->task_id);
		//mMapTasks[task_uid] = (BYTE *)malloc(sizeof(BYTE) * task_pkt_header->data_size);
		//bytecpy((BYTE *)mMapTasks[task_uid], &task[sizeof(TASK_PKT_HEADER)], task_pkt_header->data_size);
        if (0 < task_uid.first && task_uid.first < 10){
            context.mSGY->addTask(task_uid, &task[sizeof(TASK_PKT_HEADER)], task_pkt_header->data_size);
            context.mSGY->setIFAID(hostid);
        }

		printf("\tJOB ID\t\t: %d\n", task_pkt_header->job_id);
		printf("\tTASK ID\t\t: %d\n", task_pkt_header->task_id);
		printf("\tDATA SIZE\t: %d\n", task_pkt_header->data_size);
	}

	void onRecvReqTaskList(const Stigmergy::SGYContext &context,
						   const HOST_ID &from)
	{
		// std::cout << "MySGYListener::onRecvReqTaskList - come request task list" << std::endl;

		context.mSGY->sendTaskList(from);
	}

	void onRecvReqTask(const Stigmergy::SGYContext &context,
					   const JOB_ID &job_id,
					   const TASK_ID &task_id,
					   const HOST_ID &from)
	{
		// std::cout << "MySGYListener::onRecvReqTask - come request task" << std::endl;

		worker_map[from]++;

		// std::string pos = "./worker_info_1.dat";
		// std::ofstream fout(pos.c_str(), std::ios::app);
		// fout << "***" << std::endl;
		// for (auto worker_info : worker_map){
		// 	fout << worker_info.first << " - " << worker_info.second << std::endl;
		// }
		// fout << "***" << std::endl;

		context.mSGY->sendTask(from, job_id, task_id);
	}

	void onRecvTaskFin(const Stigmergy::SGYContext &context,
					   const Result &result,
					   const HOST_ID &from)
	{
		// std::cout << "MySGYListener::onRecvTaskFin - come result " << result.getJobId() << "-" << result.getTaskId() << std::endl;

		(context.mSGY)->addResult(result);

        context.mSGY->sendTaskFinToIF(context, result);

		worker_map[from]++;

		// deleting task
		std::pair<JOB_ID, TASK_ID> task_uid(result.getJobId(), result.getTaskId());
		(context.mSGY)->delTask(task_uid);
	}

	void onRecvReqResultList(const Stigmergy::SGYContext &context,
							 const HOST_ID &from)
	{
		// std::cout << "MySGYListener::onRecvReqRequestList - come result list request" << std::endl;

		std::string fname = "./woker_info_2.dat";
		std::ofstream fout(fname, std::ios::app);

		// int n = 0;
		// for (auto worker_info : worker_map){
		// 	if (n != 0){
		// 		fout << ",";
		// 	}

		// 	while (n != worker_info.first){
		// 		fout << "0,";
		// 		n++;
		// 	}
		// 	fout << worker_info.second;
		// }
        fout << worker_map.size();
		fout << std::endl;

        fout.close();

		(context.mSGY)->sendResultList(from);
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
	Stigmergy::SGYContext sgyContext(&sgy);
	cmcContext->setSGYContext(&sgyContext);

	sgy.startStigmergy();

	return (0);
}

