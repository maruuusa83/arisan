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
#include "BmpHandler.h"

namespace marusa {
namespace utilities {

BmpHandler::BmpHandler(std::string pos)
{
	BmpFile *bmpFile = new BmpFile(pos);
	
	read_header(bmpFile);
	read_info_header(bmpFile);

	read_bmp_body(bmpFile);
}

void BmpHandler::read_header(BmpFile *bmpFile)
{
	BYTE buf[BmpHandler::BUF_SIZE];

	/*** Reading file header ***/
	/* File type */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_FILE_TYPE); //ignore
	/* TODO: check file type (is bmp?) in here */

	/* File size */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_FILE_SIZE);
	this->mFileSize = *(int *)buf;
#ifdef ___DEBUG_BMP_HANDLER___
	std::cout << "File size : " << this->mFileSize << std::endl;
#endif /* ___DEBUG_BMP_HANDLER___ */

	/* Reserved area */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_RESERVED_AREA_1); //ignore
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_RESERVED_AREA_2); //ignore

	/* Offset */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_OFFSET);
	this->mOffset = *(int *)buf;
}

void BmpHandler::read_info_header(BmpFile *bmpFile)
{
	BYTE buf[BmpHandler::BUF_SIZE];

	/*** Reading Information Header ***/
	/* Size of Information header */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_INFO_HEAD);
	this->mInfoHeaderSize = *(int *)buf;

	/* Image width */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_WIDTH);
	this->mBmpWidth  = *(int *)buf;
#ifdef ___DEBUG_BMP_HANDLER___
	std::cout << "width : " << this->mBmpWidth << std::endl;
#endif /* ___DEBUG_BMP_HANDLER___ */

	/* Image height */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_HEIGHT);
	this->mBmpHeight = *(int *)buf;
#ifdef ___DEBUG_BMP_HANDLER___
	std::cout << "height : " << this->mBmpHeight << std::endl;
#endif /* ___DEBUG_BMP_HANDLER___ */

	/* Num of Plane */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_PLANE);
	this->mPlaneNum = *(int *)buf;

	/* Bit size of color */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_COLOR);
	this->mColorBit = *(int *)buf;
#ifdef ___DEBUG_BMP_HANDLER___
	std::cout << "Bolor bit : " << this->mColorBit << std::endl;
#endif /* ___DEBUG_BMP_HANDLER___ */

	/* Complession Format */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_CMP_TYPE);
	this->mCmpType = *(int *)buf;

	/* PPMH */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_PPM_H);
	this->mPPMH = *(int *)buf;

	/* PPMV */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_PPM_V);
	this->mPPMH = *(int *)buf;

	/* Coler num of palet */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_PALET);
	this->mPaletColorNum = *(int *)buf;

	/* index of important palet */
	bmpFile->get_data(buf, BmpHandler::SIZE_BMP_INDEX_IMPORTANT_PALET);
	this->mImportantPaletIndex = *(int *)buf;
}

void BmpHandler::read_bmp_body(BmpFile *bmpFile)
{
	//int size_data = sizeof(BYTE) * mBmpWidth * mBmpHeight * BmpHandler::NUM_COLOR;
	//this->mBmpData = (BYTE *)malloc(size_data);
	//bmpFile->get_data(mBmpData, size_data);

	this->mBmpData = (unsigned int **)malloc(sizeof(unsigned int *) * mBmpHeight);
	for (int i = 0; i < mBmpHeight; i++){
		mBmpData[i] = (unsigned int *)malloc(sizeof(unsigned int) * mBmpWidth);
	}

	/*  y
	 *  ^    cordinate system
	 *  |
	 *  o--->x
	 */
	bmpFile->move_to(mOffset);
	int buf_margin;
	int size_line_margin = (4 - ((mBmpWidth * (SIZE_BMP_COLOR % 8)) % 4)) % 4;
	BYTE buf[BmpHandler::BUF_SIZE];
	for (int y = 0; y < mBmpHeight; y++){
		for (int x = 0; x < mBmpWidth; x++){
			bmpFile->get_data(buf, 3);
			buf[3] = 0;

			(mBmpData[y])[x] = *(unsigned int *)buf;
		}
		bmpFile->get_data((BYTE *)&buf_margin, size_line_margin);
	}
}


/* getters and setters */
int BmpHandler::get_size(int *width, int *height)
{
	*width  = this->mBmpWidth;
	*height = this->mBmpHeight;

	return (0);
}
void BmpHandler::set_size(const int &width, const int &height)
{
	this->mBmpWidth  = width;
	this->mBmpHeight = height;
}



int BmpHandler::get_pixel(BYTE rgb[BmpHandler::NUM_COLOR], int pos_x, int pos_y)
{
	if (this->mBmpWidth < pos_y || this->mBmpHeight < pos_x){
		return (-1);
	}

	//for (int i = 0; i < BmpHandler::NUM_COLOR; i++){
	//	rgb[i] = this->mBmpData[pos_x * pos_y * BmpHandler::NUM_COLOR + i];
	//}

	for (int i = 0; i < BmpHandler::NUM_COLOR; i++){
		rgb[i] = ((char *)&(mBmpData[pos_y][pos_x]))[i];
	}

	return (mBmpData[pos_y][pos_x]);
}
int BmpHandler::set_pixel(const BYTE rgb[3], const int &pos_x, const int &pos_y)
{
	if (this->mBmpWidth < pos_y || this->mBmpHeight < pos_x){
		return (-1);
	}

	mBmpData[pos_y][pos_x] = *(unsigned int *)rgb;

	return (mBmpData[pos_y][pos_x]);
}

void BmpHandler::generate_bmp(std::string pos)
{
	int iTmp;

	std::ofstream *fout;
	fout = new std::ofstream(pos.c_str(), std::ios::out | std::ios::binary);

#ifdef ___DEBUG_BMP_HANDLER___
	std::cout << "*** GENERATE BMP FILE ***" << std::endl;
#endif /* ___DEBUG_BMP_HANDLER___ */

	int w, h;
	get_size(&w, &h);

	/* -Calculation of file size-
	 * Line of image data must be multiples of 4.
	 * When line isn't multiples of 4,
	 * line have mergin that is some 0x0000 in the end of it.
	 */
	int size_img_data = w * (mColorBit / 8) * h;
#ifdef ___DEBUG_BMP_HANDLER___
	std::cout << "width : " << w << ", height : " << h << std::endl;
	printf("Size of BODY : %d * %d * %d = %d\n", w, h, (mColorBit / 8), size_img_data);
#endif /* ___DEBUG_BMP_HANDLER___ */
	int size_line_margin = 0;
	if (((w * (SIZE_BMP_COLOR % 8)) % 4) != 0){
		size_line_margin = (4 - ((w * (mColorBit / 8)) % 4));
		size_img_data += size_line_margin * h;
	}
#ifdef ___DEBUG_BMP_HANDLER___
	std::cout << "line margin : " << size_line_margin << std::endl;
#endif /* ___DEBUG_BMP_HANDLER___ */
	
	/*** File header ***/
	/* File type */
	fout->write("BM", SIZE_BMP_FILE_TYPE);

	/* File size */
	iTmp = size_img_data + 54; //Image data size + Header size
	fout->write((char *)&iTmp, SIZE_BMP_FILE_SIZE);

	/* Reserved area 1 */
	iTmp = 0x00;
	fout->write((char *)&iTmp, SIZE_BMP_RESERVED_AREA_1);

	/* Reserved area 2 */
	iTmp = 0x00;
	fout->write((char *)&iTmp, SIZE_BMP_RESERVED_AREA_2);

	/* Offset to data */
	iTmp = 0x36;
	fout->write((char *)&iTmp, SIZE_BMP_OFFSET);

	/*** Information Header ***/
	/* Size of Info header */
	iTmp = 0x28;
	fout->write((char *)&iTmp, SIZE_BMP_INFO_HEAD);

	/* Width and Height of image */
	fout->write((char *)&w, SIZE_BMP_WIDTH);
	fout->write((char *)&h, SIZE_BMP_HEIGHT);

	/* Number of plane */
	iTmp = 0x01; //Anytime, 1.
	fout->write((char *)&iTmp, SIZE_BMP_PLANE);

	/* Color bits */
	iTmp = 24; //24bit color
	fout->write((char *)&iTmp, SIZE_BMP_COLOR);

	/* Compression type */
	iTmp = 0x00; //No compression
	fout->write((char *)&iTmp, SIZE_BMP_CMP_TYPE);

	/* Size of image data */
	fout->write((char *)&size_img_data, SIZE_BMP_SIZE);

	/* PPM */
	iTmp = 0x01;
	fout->write((char *)&iTmp, SIZE_BMP_PPM_H);
	fout->write((char *)&iTmp, SIZE_BMP_PPM_V);

	/* about palet */
	iTmp = 0x00;
	fout->write((char *)&iTmp, SIZE_BMP_PALET);
	fout->write((char *)&iTmp, SIZE_BMP_INDEX_IMPORTANT_PALET);

	/*** Image data ***/
#ifdef ___DEBUG_BMP_HANDLER___
	unsigned int hoge = 0;
#endif /* ___DEBUG_BMP_HANDLER___ */
	for (int y = 0; y < mBmpHeight; y++){
		for (int x = 0; x < mBmpWidth; x++){
			fout->write((char *)&(mBmpData[y][x]), 3);
#ifdef ___DEBUG_BMP_HANDLER___
			hoge += 3;
#endif /* ___DEBUG_BMP_HANDLER___ */
		}
		iTmp = 0x00;
		fout->write((char *)&iTmp, size_line_margin);
#ifdef ___DEBUG_BMP_HANDLER___
		hoge += size_line_margin;
#endif /* ___DEBUG_BMP_HANDLER___ */
	}
	fout->close();
#ifdef ___DEBUG_BMP_HANDLER___
	printf("finish : %d bytes\n", hoge + 54);
#endif /* ___DEBUG_BMP_HANDLER___ */
}


/* BmpHandler::BmpFile class */
BmpHandler::BmpFile::BmpFile(std::string pos)
{
#ifdef ___DEBUG_BMP_HANDLER___
	std::cout << "File open : " << pos << std::endl;
#endif /* ___DEBUG_BMP_HANDLER___ */
	this->mFin = new std::ifstream(pos.c_str(), std::ios::in | std::ios::binary);

	if (!(this->mFin)){
		// TODO: if file can't open, error will be occured
		return;
	}
}

void BmpHandler::BmpFile::get_data(BYTE *buf, int size)
{
	BYTE *pos;

	pos = buf;
	for (int i = 0; i < size; i++){
		mFin->read((char *)pos, 1);
		pos++;
	}
}

void BmpHandler::BmpFile::move_to(const unsigned int &pos)
{
	mFin->seekg(pos);
}


} /* namespace utilities */
} /* namespace marusa */
