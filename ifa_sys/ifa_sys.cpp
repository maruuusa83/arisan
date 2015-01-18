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
#include "InterfaceAppAPI.h"

#include "CmcAdapter.h"
#include "CmcContext.h"
#include "Job.h"

#include "../mycmc/MyCmc.h"

using std::cout;
using std::endl;

using marusa::swms::InterfaceAppAPI;
using marusa::swms::JOB_ID;
using marusa::swms::HOST_ID;
using marusa::swms::BYTE;

using marusa::swms::Job;

class MyIFAListener : public InterfaceAppAPI::IFACallbackListener
{
public:
	void onRecvJobId(const InterfaceAppAPI::IFAContext &context,
					 const JOB_ID &job_id){
		cout << "MyIFAListener::onRecvJobId : job_id = " << job_id << endl;
	}
};

int main()
{
	/*** Initialize ***/
	MyIFAListener *listener = new MyIFAListener();
	CmcAdapter::CmcCallbackListener *cmcCL = new CmcAdapter::CmcCallbackListener();

	CmcAdapter::CmcContext *cmcContext = new CmcAdapter::CmcContext();
	cmcContext->setIFACallbackListener(listener);
	MyCmc *cmc = new MyCmc(cmcContext, cmcCL);


	/*** sending test data ***/
	BYTE data[] = "This is test data";

	Job::Task task;
	task.setData(data, sizeof(data));

	Job job;
	job.addTask(task);

	InterfaceAppAPI ifa(listener, cmc);
	ifa.sendTasks(job);

	return (0);
}

