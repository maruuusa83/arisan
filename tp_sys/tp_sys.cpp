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
#include "TaskProcessorAPI.h"

#include "CmcAdapter.h"
#include "CmcContext.h"

#include "Job.h"
#include "Result.h"

#include "../mycmc/MyCmc.h"

using std::cout;
using std::endl;

using marusa::swms::TaskProcessorAPI;
using marusa::swms::CmcAdapter;
using marusa::swms::Job;
using marusa::swms::Result;

using marusa::swms::bytecpy;

using marusa::swms::JOB_ID;
using marusa::swms::TASK_ID;
using marusa::swms::TASK_INFO;
using marusa::swms::TASK_PKT_HEADER;
using marusa::swms::TASKLST_PKT_BODY;


class MyTPListener : public TaskProcessorAPI::TPCallbackListener
{
	void onTask(const TaskProcessorAPI::TPContext &context,
				const Job::Task &task)
	{
		cout << "MyTPListener::onTask - on task" << endl;

		Result result(task.getJobId(), task.getTaskId(), nullptr, 0);
		(context.taskProcessorAPI)->sendTaskFin(result);
	}

	void onTaskList(const TaskProcessorAPI::TPContext &context,
					const std::vector<TASKLST_PKT_BODY *> &tasklist)
	{
		cout << "MyTPListener::onTaskList - task list :" << endl;

		std::map<std::pair<JOB_ID, TASK_ID>, TASK_INFO *> newTaskList;
		for (auto task_info : tasklist){
			cout << "\t";
			cout << "JOB-" << task_info->job_id << " ";
			cout << "TASK-" << task_info->task_id << " ";
			cout << ": " << task_info->put_time << endl;

			std::pair<JOB_ID, TASK_ID> task_uid(task_info->job_id, task_info->task_id);

			TASK_INFO *info = (TASK_INFO *)malloc(sizeof(TASK_INFO));
			info->job_id    = task_info->job_id;
			info->task_id   = task_info->task_id;
			info->put_time  = task_info->put_time;
			info->task_data = nullptr;

			newTaskList[task_uid] = info;
		}

		(context.taskProcessorAPI)->renewTaskList(newTaskList);
	}
};

int main()
{
	MyTPListener *listener = new MyTPListener();

	CmcAdapter::CmcCallbackListener *cmcCL = new CmcAdapter::CmcCallbackListener();
	CmcAdapter::CmcContext *cmcContext = new CmcAdapter::CmcContext();
	MyCmc *cmc = new MyCmc(cmcContext, cmcCL);

	TaskProcessorAPI tp(listener, cmc);
	TaskProcessorAPI::TPContext tpContext(&tp);
	cmcContext->setTPContext(&tpContext);
	cmcContext->setTPCallbackListener(listener);

	tp.startWorker();

	return (0);
}


