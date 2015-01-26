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

#include <time.h>
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
using marusa::swms::bytecpy;

using marusa::swms::Job;

using marusa::utilities::BmpHandler;

static const int BUF_SIZE = 128;

static const int NUM_EX_JOB = 1;
static const int NUM_DIV_IMG = 81;
bool fin_flag;

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

		if (results_info.size() == NUM_DIV_IMG * NUM_EX_JOB){
			fin_flag = true;
		}
	}
};


int splitBmpN(std::vector<BmpHandler> &dividedBmps, const BmpHandler &bmp, const int &N)
{
	int width, height;
	bmp.get_size(&width, &height);

	cout << "splitBmpNine - Split to w:" << width / N << " h:" << height / N << endl;

	for (int y = 0; y < height; y += height / N){
		for (int x = 0; x < width; x += width / N){
			cout << "\tBLK (" << x << ", " << y << ")" << endl;
			BmpHandler miniBmp;

			// cpy data size
			miniBmp.set_size(width / N, height / N);

			// cpy data body
			miniBmp.init_canbus();
			BYTE rgb_tmp[N];
			for (int pos_y = y; pos_y < y + (height / N); pos_y++){
				for (int pos_x = x; pos_x < x + (width / N); pos_x++){
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

int sendBmpAsJob(InterfaceAppAPI &ifa, const BmpHandler &bmp, const JOB_ID &job_id)
{
	std::vector<BmpHandler> dividedBmps;
	splitBmpN(dividedBmps, bmp, 9);

	Job job(job_id);
	for (auto dividedBmp : dividedBmps){
		int width, height;
		dividedBmp.get_size(&width, &height);

		BYTE *task_data = (BYTE *)malloc(sizeof(TASK_HEADER) + (width * height * 3));

		// build pkt header
		((TASK_HEADER *)task_data)->width = width;
		((TASK_HEADER *)task_data)->height = height;

		((TASK_HEADER *)task_data)->data_size = width * height * 3;

		// cpy pic data
		BYTE pixel[3];
		BYTE *pos = task_data + sizeof(TASK_HEADER);
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				dividedBmp.get_pixel(pixel, x, y);
				bytecpy(pos, pixel, 3);
				pos += 3;
			}
		}

		Job::Task task;
		task.setData(task_data, sizeof(TASK_HEADER) + (width * height * 3));

		job.addTask(task);
	}

	ifa.sendTasks(job);

	return (0);
}

int sendJobFromJobFile(InterfaceAppAPI &ifa, const std::string jobfile_path)
{
	std::ifstream fin(jobfile_path.c_str(), std::ios::in);
	char buf[BUF_SIZE];

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

		sendBmpAsJob(ifa, bmp, job_id);
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

	Job job(0);
	job.addTask(task);

	InterfaceAppAPI ifa(listener, cmc);
	ifa.sendTasks(job);

	std::string jobfile_path;
	while (1){
		printf("0:sendReqResultList\n");
		printf("1:read and send Job from Job file\n");
		printf("2:start experiment\n");
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

			sendJobFromJobFile(ifa, jobfile_path);
			break;

		  case 2:
		  {
			printf("Start Experiment\n");
			printf("Input path for Job file : ");
			std::cin >> jobfile_path;

			cout << jobfile_path << endl;


			/******************************/
			/***       experiment      ****/
			struct timeval clk_start, clk_end;

			gettimeofday(&clk_start, NULL);
			sendJobFromJobFile(ifa, jobfile_path);
			while (1){
				ifa.sendReqResultList();

				if (fin_flag == true){
					break;
				}

				sleep(1);
			}

			gettimeofday(&clk_end, NULL);
			printf("***FINISH EXPERIMENT***\n");

			struct timeval tv_result;
			tv_result.tv_sec  = clk_end.tv_sec - clk_start.tv_sec;
			if (clk_start.tv_usec <= clk_end.tv_usec){
				tv_result.tv_usec = clk_end.tv_usec - clk_start.tv_usec;
			}
			else {
				tv_result.tv_sec--;
				tv_result.tv_usec = clk_start.tv_usec - clk_end.tv_usec;
			}

			printf("time : %f\n", tv_result.tv_sec * 1000. + tv_result.tv_usec * 1000.);
			/***   experiment for here  ***/
			/******************************/
			break;
		  }

		  case 9:
			return (0);

		  default:
			break;
		}
	}
}

