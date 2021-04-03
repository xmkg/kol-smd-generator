/**
 * ______________________________________________________
 * This file is part of ko-smd-generator project.
 * 
 * @author       Mustafa Kemal Gılor <mustafagilor@gmail.com> (2016)
 * .
 * SPDX-License-Identifier:	MIT
 * ______________________________________________________
 */

#include "stdafx.h"
#include "ObjectPostData.h"
#include <vector>

#define CRY_KEY 0x0816


CObjectPostData::CObjectPostData() :m_bOldFileMode(false) {}
CObjectPostData::~CObjectPostData() {}


bool CObjectPostData::Load(const std::string& szPath)
{
	uint32  iUnknown = 0;
	FILE * fp = nullptr;

	TRACE("CObjectPostData::Load() - Loading file %s.", szPath.c_str());

	/* Old opd format */
	if(m_bOldFileMode)
		fread_s(&iUnknown, 4, 1, 4, fp);

	/* Open for reading, in binary mode */
	fopen_s(&fp, szPath.c_str(), "rb");

	if (fp == nullptr)
	{
		TRACE("CObjectPostData::Load() - The file %s does not exist or the permission is not enough to access it.", szPath.c_str());
		return false;
	}

	/* Read map name from file */
	auto szMapName = ReadDecryptString(fp);
	printf(" * real map name is %s * \n", szMapName.c_str());

	/*
	This part is unknown, usual value is zero (maybe it's a non-used description string)
	(does not exist on versions below 1534)
	*/
	if (!m_bOldFileMode)
		fread_s(&iUnknown, 4, 1, 4, fp);

	/* Load the file */
	bool bSuccess = m_ShapeMgr.Load(fp, m_bOldFileMode);
	

	TRACE("CObjectPostData::Load() - Zone name %s, %gx%g, iUnknown = %d, Collision face count = %d", szMapName.c_str(), m_ShapeMgr.Width(), m_ShapeMgr.Height(), iUnknown, m_ShapeMgr.ColFaceCount());
	fclose(fp);
	return bSuccess;
}

void CObjectPostData::DoCleanup()
{
	/* Cleanup shape data */
	m_ShapeMgr.DoCleanup();
}

static std::string EncryptPassword(std::string & pass)
{

	static const char key[100] = { 0x1A, 0x1F, 0x11, 0xA, 0x1E, 0x10, 0x18, 0x2, 0x1D, 0x8, 0x14, 0xF, 0x1C, 0xB, 0xD, 0x4, 0x13, 0x17, 0x0, 0xC, 0xE, 0x1B, 0x6, 0x12, 0x15, 0x3, 0x9, 0x7, 0x16, 0x1, 0x19, 0x5, 0x12, 0x1D, 0x7, 0x19, 0xF, 0x1F, 0x16, 0x1B, 0x9, 0x1A, 0x3, 0xD, 0x13, 0xE, 0x14, 0xB, 0x5, 0x2, 0x17, 0x10, 0xA, 0x18, 0x1C, 0x11, 0x6, 0x1E, 0x0, 0x15, 0x8, 0x4, 0x1 };
	static const char hash[40] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	unsigned int  intPass, tmp, destCounter = 0, uzunluk = strlen(pass.c_str());
	char passExt[40] = "";
	char buffer[40] = "";
	if ((uzunluk % 4) != 0) uzunluk = uzunluk + 4 - (uzunluk % 4);
	strcpy(passExt, pass.c_str());

	for (int i = 0; i < uzunluk / 4; i++) {
		intPass = 0;
		tmp = *reinterpret_cast<unsigned int*>(passExt + i * 4) + 0x3e8;
		int counter = 0;
		while (tmp) {
			if (tmp % 2) intPass += 1 << key[counter];
			tmp = tmp >> 1;
			counter++;
		}
		for (int j = 0; j < 7; j++)
		{
			unsigned long long mulres = unsigned long long(intPass) * unsigned long long(0x38E38E39);
			unsigned __int32 high = (mulres & 0xFFFFFFFF00000000ULL) >> 32;
			//unsigned __int32 low = mulres & 0xFFFFFFFF;

			tmp = high >> 3;
			buffer[destCounter] = hash[intPass - ((tmp * 9) << 2)];
			intPass = tmp;
			destCounter++;
		}
	}
	return std::string(buffer);
}

std::string DecryptString(const std::string & encStr)
{
	const unsigned short CipherKey1 = 0x6081;
    const unsigned short CipherKey2 = 0x1608;
	char buffer[512] = "";


	unsigned short _volatileKey = 0x0816;
	for (int i = 0; i < encStr.length()-1; i++)
	{
		unsigned __int8 rawByte = encStr[i];
		unsigned __int8 temporaryKey = (unsigned __int8)((_volatileKey & 0xff00) >> 8);
		unsigned __int8 encryptedByte = (unsigned __int8)(temporaryKey ^ rawByte);
		_volatileKey = (unsigned short)((rawByte + _volatileKey)*CipherKey1 + CipherKey2);
		buffer[i] = encryptedByte;
	}
	// null terminator

	return std::string(buffer);
}

std::string CObjectPostData::ReadDecryptString(FILE* fp)
{


	//DWORD dwNum;
	int iCount;

	fread_s(&iCount, sizeof(int), sizeof(int), 1, fp);

	if (iCount == 1)
	{
		m_bOldFileMode = true;
		// OLD file format
		// Read again to get the size
		fread_s(&iCount, sizeof(int), sizeof(int), 1, fp);
	}
	std::vector<char> buffer(iCount);

	fread_s(&(buffer[0]), iCount, 1, iCount, fp);				// string
	/*for (auto i = 0; i < iCount; i++)
		buffer[i] ^= CRY_KEY;*/
	buffer.push_back(static_cast<char>(0x00));

	std::string strDest(buffer.begin(), buffer.end());
	/* Old file names are not encrypted. */
	if (m_bOldFileMode)
		return strDest;

	std::string decStr = DecryptString(strDest);
	//std::string xhamster = EncryptPassword(strDest);
	//strDest = buffer.begin();

	return decStr;
}