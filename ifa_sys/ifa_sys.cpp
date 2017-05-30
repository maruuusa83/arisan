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

#include <vector>
#include <array>

#include <time.h>
#include <sys/time.h>
#include <string>

#include <chrono>
#include <random>

#include "../RC4/rc4.h"

#include "common.h"
#include "InterfaceAppAPI.h"
#include "Result.h"

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
using marusa::swms::Result;

using marusa::utilities::BmpHandler;

static const int BUF_SIZE = 128;

static const int NUM_EX_JOB = 1;
constexpr int NUM_SPLIT = 9;
constexpr int NUM_DIV_IMG = NUM_SPLIT * NUM_SPLIT;
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

	}

    void onRecvTaskFin(const InterfaceAppAPI::IFAContext &context,
                       const Result &result)
    {
        BYTE *data;
        unsigned int data_size;
        static int cnt = 0;

        result.getData(&data, data_size);

        cout << "MyIFAListener::onRecvTaskFin - task fin";
        printf(" %d %d\n", data[0], ++cnt);

        if (data[0] == 1){
            cout << "MyIFAListener::onRecvTaskFin - A KEY HAS FOUND !!!";
            fin_flag = true;

            cout << endl;
            for (unsigned int i = 1; i < data_size; i++){
                printf("%d ", data[i]);
            }
            cout << endl;
        }


        result.freeData(data);
    }
};


int splitBmpN(std::vector<BmpHandler> &dividedBmps, const BmpHandler &bmp, const int &N)
{
	int width, height;
	bmp.get_size(&width, &height);

	cout << "splitBmpNine - Split to w:" << width / N << " h:" << height / N << endl;

	for (int y = 0; y + (height / N) < height; y += height / N){
		for (int x = 0; x + (width / N)< width; x += width / N){
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
	splitBmpN(dividedBmps, bmp, NUM_SPLIT);

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

int sendJob(InterfaceAppAPI &ifa, unsigned int num_split, std::array<marusa::BYTE, KEY_SIZE> &key, std::vector<marusa::BYTE> &plain_text)
{
    static int job_id = 0;

    // Job Settings
    std::random_device rd;
    std::mt19937 mt(rd()); // give a seed using std::random_device
    std::uniform_int_distribution<> rand_byte(0, marusa::BYTE_SIZE - 1);

    for (unsigned int i = 0; i < KEY_SIZE; i++) key[i] = rand_byte(mt) % marusa::BYTE_SIZE;

    for (unsigned int i = 0; i < TEXT_SIZE; i++){
        plain_text.push_back(rand_byte(mt));
    }

    marusa::RC4<KEY_SIZE> rc4(key);
    std::vector<marusa::BYTE> cipher_text;
    rc4.exec(plain_text, cipher_text);

    unsigned int t_start_position = 0;

    // Generate Job
    Job job(++job_id);
    for (unsigned int task_id = 0; task_id < num_split; task_id++){
        TASK_RC4_ATK task_data;

        for (unsigned int i = 0; i < TEXT_SIZE; i++){
            task_data.plain_text[i] = plain_text[i];
            task_data.cipher_text[i] = cipher_text[i];
        }

        for (unsigned int i = 0; i < KEY_SIZE; i++){
            task_data.from[i] = ((BYTE *)&t_start_position)[i];
        }
        task_data.split_size = (unsigned int)CALC_SPLIT_SIZE(num_split);
        
        Job::Task task;
        task.setData((BYTE *)&task_data, sizeof(TASK_RC4_ATK));
        job.addTask(task);

        t_start_position += CALC_SPLIT_SIZE(num_split);
    }

    ifa.sendTasks(job);

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

        std::array<marusa::BYTE, KEY_SIZE> key;
        std::vector<marusa::BYTE> plain_text;

		switch (cmd){
		  case 0:
			ifa.sendReqResultList();
			break;

		  case 1:
            sendJob(ifa, 16, key, plain_text);
			break;

		  case 2:
		  {
			printf("Start Experiment\n");
            printf("SPLIT NUM ? : ");
            int split_num;
            scanf("%d", &split_num);
            

			/******************************/
			/***       experiment      ****/
            auto start = std::chrono::system_clock::now();

            fin_flag = false;
            sendJob(ifa, split_num, key, plain_text);

			while (1){
				if (fin_flag == true){
					break;
				}

                ifa.sendReqResultList();

				usleep(100000);
			}

            auto end = std::chrono::system_clock::now();
			printf("#######***FINISH EXPERIMENT***########\n");

#define PRINT_BYTES(DATA) for (auto byte : (DATA)) printf("%d ", byte)
            PRINT_BYTES(key);
#undef PRINT_BYTES

            auto diff = end - start;
            std::cout << "time : "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
                      << " msec" << std::endl;
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

