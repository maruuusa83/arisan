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
#ifndef ___BMPHANDLER_H___
#define ___BMPHANDLER_H___

#include "common.h"

#include <string>
#include <fstream>
#include <stdlib.h>

#define ___DEBUG_BMP_HANDLER___
#ifdef ___DEBUG_BMP_HANDLER___
#include <iostream>
#include <stdio.h>
#endif /* ___DEBUG_BMP_HANDLER___ */

namespace marusa {
namespace utilities {

using marusa::swms::BYTE;

class BmpHandler
{
private:
	class BmpFile;

public:
	BmpHandler();
	BmpHandler(std::string pos);

	int get_size(int *width, int *height) const;
	void set_size(const int &width, const int &height);

	int get_pixel(BYTE rgb[3], int pos_x, int pos_y) const;
	int set_pixel(const BYTE rgb[3], const int &pos_x, const int &pos_y);

	void generate_bmp(std::string pos);

	static const int NUM_COLOR = 3;

private:
	int mFileSize;
	int mOffset;

	int mInfoHeaderSize;
	int mBmpWidth, mBmpHeight;
	int mPlaneNum;
	int mColorBit;
	int mCmpType;
	int mPPMH, mPPMV;
	int mPaletColorNum, mImportantPaletIndex;

	unsigned int **mBmpData;


	void read_header(BmpFile *bmpFile);
	void read_info_header(BmpFile *bmpFile);
	void read_bmp_body(BmpFile *bmpFile);


	class BmpFile
	{
	public:
		BmpFile(std::string pos);

		void get_data(BYTE *buf, int size);
		void move_to(const unsigned int &pos);

	private:
		std::ifstream *mFin;
	};

	static const int COL_R = 0, COL_G = 1, COL_B = 2;

	static const int BUF_SIZE = 8;

	static const int SIZE_BMP_FILE_TYPE	= 2;
	static const int SIZE_BMP_FILE_SIZE	= 4;
	static const int SIZE_BMP_RESERVED_AREA_1 = 2;
	static const int SIZE_BMP_RESERVED_AREA_2 = 2;
	static const int SIZE_BMP_OFFSET	= 4;

	static const int SIZE_BMP_INFO_HEAD	= 4;
	static const int SIZE_BMP_WIDTH		= 4;
	static const int SIZE_BMP_HEIGHT	= 4;
	static const int SIZE_BMP_PLANE		= 2;
	static const int SIZE_BMP_COLOR		= 2;
	static const int SIZE_BMP_CMP_TYPE	= 4;
	static const int SIZE_BMP_SIZE		= 4;
	static const int SIZE_BMP_PPM_H		= 4;
	static const int SIZE_BMP_PPM_V		= 4;
	static const int SIZE_BMP_PALET		= 4;
	static const int SIZE_BMP_INDEX_IMPORTANT_PALET = 4;

};



} /* utilities */
} /* marusa */

#endif /* ___BMPHANDLER_H___ */
