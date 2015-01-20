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

#include "../mycmc/MyCmc.h"

using std::cout;
using std::endl;

using marusa::swms::TaskProcessorAPI;
using marusa::swms::CmcAdapter;
using marusa::swms::Job;

using marusa::swms::bytecpy;

using marusa::swms::JOB_ID;
using marusa::swms::TASK_ID;
using marusa::swms::TASK_PKT_HEADER;


class MyTPListener : public TaskProcessorAPI::TPCallbackListener
{
	void onTask(const TaskProcessorAPI::TPContext &context,
				const Job::Task &task)
	{
		cout << "MyTPListener::onTask - on task" << endl;
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

	tp.startWorker();

	return (0);
}


