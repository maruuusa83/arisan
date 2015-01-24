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
#include <unistd.h>

#include <string>

#include "common.h"
#include "InterfaceAppAPI.h"

#include "CmcAdapter.h"
#include "CmcContext.h"
#include "Job.h"

#include "../mycmc/MyCmc.h"

#include "../utilities/BmpHandler.h"

using std::cout;
using std::endl;

using marusa::swms::InterfaceAppAPI;
using marusa::swms::JOB_ID;
using marusa::swms::TASK_ID;
using marusa::swms::HOST_ID;
using marusa::swms::BYTE;

using marusa::swms::Job;

using marusa::utilities::BmpHandler;

static const int BUF_SIZE = 128;

class MyIFAListener : public InterfaceAppAPI::IFACallbackListener
{
public:
	void onRecvJobId(const InterfaceAppAPI::IFAContext &context,
					 const JOB_ID &job_id){
		cout << "MyIFAListener::onRecvJobId : job_id = " << job_id << endl;
	}

	void onRecvResultList(const InterfaceAppAPI::IFAContext &context,
						  const std::vector<std::pair<JOB_ID, TASK_ID>> &results_info)
	{
		cout << "MyIFAListener::onRecvResultList - result list :" << endl;

		for (auto info : results_info){
			printf("\tJOB-%d TASK-%d\n", info.first, info.second);
		}
	}
};


int splitBmpNine(std::vector<BmpHandler> &dividedBmps, const BmpHandler &bmp)
{
	int width, height;
	bmp.get_size(&width, &height);

	cout << "splitBmpNine - Split to w:" << width / 3 << " h:" << height / 3 << endl;

	for (int y = 0; y < height; y += height / 3){
		for (int x = 0; x < width; x += width / 3){
			BmpHandler miniBmp;

			// cpy data size
			miniBmp.set_size(width / 3, height / 3);

			// cpy data body
			BYTE rgb_tmp[3];
			for (int pos_y = y; pos_y < y + (height / 3); pos_y++){
				for (int pos_x = x; pos_x < x + (width / 3); pos_x++){
					bmp.get_pixel(rgb_tmp, pos_x, pos_y);
					miniBmp.set_pixel(rgb_tmp, pos_x - x, pos_y - y);
				}
			}

			// add to vector
			dividedBmps.push_back(miniBmp);
		}
	}

	return (0);
}

int sendBmpAsJob(const BmpHandler &bmp, const JOB_ID &job_id)
{
	std::vector<BmpHandler> dividedBmps;
	splitBmpNine(dividedBmps, bmp);

	for (auto dividedBmp : dividedBmps){

	}

	return (0);
}

int sendJobFromJobFile(const std::string jobfile_path)
{
	std::ifstream fin(jobfile_path.c_str(), std::ios::in);
	char buf[BUF_SIZE];
	int job_id;

	while (1){
		fin.getline(buf, BUF_SIZE);
		if (buf[0] == '0' && buf[1] == '\0'){
			// end of input
			break;
		}
		int i = 0;
		while (buf[i] != '\0') i++;
		i--;
		int job_id = 0;
		while (i >= 0){
			job_id *= 10;
			job_id += buf[i] - '0';
			i--;
		}
		printf("new task - JOB_ID : %d\n", job_id);


		fin.getline(buf, BUF_SIZE);
		std::string pos(buf);
		BmpHandler bmp(pos);

		sendBmpAsJob(bmp, job_id);
	}

	return (0);
}


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


	std::string jobfile_path;
	while (1){
		printf("0:sendReqResultList\n");
		printf("1:read and send Job from Job file\n");
		printf("9:quit this program\n");

		int cmd = 0;
		scanf("%d", &cmd);

		switch (cmd){
		  case 0:
			ifa.sendReqResultList();
			break;

		  case 1:
			printf("Input path for Job file : ");
			std::cin >> jobfile_path;

			cout << jobfile_path << endl;

			sendJobFromJobFile(jobfile_path);
			break;

		  case 9:
			return (0);

		  default:
			break;
		}
	}
}

