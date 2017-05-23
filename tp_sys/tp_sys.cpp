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
#include <random>
#include <vector>

#include "../RC4/rc4.h"
#include "../settings_ieice.h"

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
		if ((context.taskProcessorAPI)->getForbidInteruptFlag()){
			return;
		}
		else {
			(context.taskProcessorAPI)->forbidInterupt();
		}

		/*** Task Processing ***/
		cout << "MyTPListener::onTask - on task " << " " << task.getJobId() << "-" << task.getTaskId() << endl;

		BYTE *data;
		unsigned int data_size;
		task.getData(&data, data_size);
	        for (unsigned int i = 0; i < data_size; i++) printf("%c ", data[i]);
	        // attack(plain, cipher, obtained_key, from, split_size);

		/*** Send Result ***/
		Result result(task.getJobId(), task.getTaskId(), nullptr, 0);
		(context.taskProcessorAPI)->sendTaskFin(result);

		cout << "\ttask fin:" << endl;

		(context.taskProcessorAPI)->permitInterupt();
	}

	void onTaskList(const TaskProcessorAPI::TPContext &context,
					const std::vector<TASKLST_PKT_BODY *> &tasklist)
	{
		cout << "MyTPListener::onTaskList - task list :" << endl;

		std::map<std::pair<JOB_ID, TASK_ID>, TASK_INFO *> newTaskList;
		for (auto task_info : tasklist){
			/*
			cout << "\t";
			cout << "JOB-" << task_info->job_id << " ";
			cout << "TASK-" << task_info->task_id << " ";
			cout << ": " << task_info->put_time << endl;
			*/

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

	unsigned int attack(const std::vector<marusa::BYTE> &plain,
                            const std::vector<marusa::BYTE> &cipher,
                            std::array<marusa::BYTE, KEY_SIZE> &obtained_key,
                            const std::array<marusa::BYTE, KEY_SIZE> &from,
                            const unsigned int split_size)
	{
	    /*** Key Attack ***/
	    obtained_key = from;
	
	    std::vector<marusa::BYTE> t_dec;
	    unsigned int trip_count = 0;
	    while (true){
	        if (trip_count == split_size){
	            return (trip_count);
	        }
	 #ifdef ___DEBUG___
	        if (trip_count % PRINT_COUNT == 0){
	            printf("Tried %d keys\n", trip_count);
	        }
	 #endif /* ___DEBUG___ */
	
	        /* try key */
	        marusa::RC4<KEY_SIZE> t_rc4(obtained_key);
	        t_rc4.exec(cipher, t_dec);
	
	        if (t_dec == plain){ // when find the key
	            break;
	        }
	
	        /* increment key */
	        for (int i = 0; i < (int)obtained_key.size(); i++){
	            if (obtained_key[i] == 255){
	                obtained_key[i] = 0;
	            }
	            else {
	                obtained_key[i]++;
	                break;
	            }
	        }
	
	        trip_count++;
	    }
	
	    // printf("%d\n", trip_count);
	    return (trip_count);
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


