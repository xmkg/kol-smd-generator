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
#include "GameTerrain.h"
#include <vector>

/*
	We don't have to load the whole game terrain data.
	The height map is enough for us.
*/

CGameTerrain::CGameTerrain()
{

}


CGameTerrain::~CGameTerrain()
{
}

bool CGameTerrain::Load(const std::string& szGtdFile)
{
	FILE * fp = nullptr;
	fopen_s(&fp, szGtdFile.c_str(), "rb");
	if(fp == nullptr)
	{
		TRACE("CGameTerrain::Load() - GTD file does not exist.");
		return false;
	}

	LoadFromStream(fp,true);

	fclose(fp);

	return true;
}

bool CGameTerrain::LoadFromStream(FILE* fp,bool readingGtd)
{
	if (readingGtd)
	{
		uint32 nStringSize, nUnknown;
		fread_s(&nStringSize, 4, 4, 1, fp);
		std::string mapname;
		if (nStringSize > 2)
		{
			// New file format
			std::vector<char> buffer(nStringSize + 1, NULL);
			fread_s(&(buffer[0]), nStringSize, 1, nStringSize, fp);
			std::string n(buffer.begin(), buffer.end());
			mapname = n;

			/* Unknown value, only exist in new format*/
			fread_s(&nUnknown, 4, 4, 1, fp);
		}
		else
		{
			// Read string size
			fread_s(&nStringSize, 4, 4, 1, fp);
			// old file format
			std::vector<char> buffer(nStringSize + 1, NULL);
			fread_s(&(buffer[0]), nStringSize, 1, nStringSize, fp);
			std::string n(buffer.begin(), buffer.end());
			mapname = n;

		}
	}


	fread_s(&m_iHeightMapSize, 4, 4, 1, fp);
	if (!readingGtd)
		fread_s(&m_fUnitDistance, 4, 4, 1, fp);
	//fseek()
	//nHeightMapSize--;

	//	int expandSize = (nHeightMapSize - 1) * 4;
	// Initialize heightmap
	m_ppHeights = new float*[m_iHeightMapSize];

	for (int i = 0; i < m_iHeightMapSize; i++)
	{
		m_ppHeights[i] = new float[m_iHeightMapSize];

	}


	for (int z = 0; z < m_iHeightMapSize; z++)
	{
		for (int x = 0; x < m_iHeightMapSize; x++)
		{
			fread_s(&m_ppHeights[x][z], 4, 4, 1, fp);
			// skip dxtid
			if (readingGtd)
				fseek(fp, 4, SEEK_CUR);
		}
	}
	return true;
}

void CGameTerrain::SaveToFilestream(FILE* fp)
{
	const static float fUnitDistance = 4.0f;
	fwrite(&m_iHeightMapSize, 4, 1, fp);
	fwrite(&fUnitDistance, 4, 1, fp);
	/* Write terrain data to filestream */
	for (int z = 0; z < m_iHeightMapSize; z++)
	{
		for (int x = 0; x < m_iHeightMapSize; x++)
		{
			fwrite(&m_ppHeights[x][z], 4, 1, fp);
		}
	}
}

float CGameTerrain::GetHeight(int fX, int fZ)
{
	if (fX >= m_iHeightMapSize || fZ >= m_iHeightMapSize || fX < 0 || fZ < 0)
		return 0.0f;
	return m_ppHeights[fX][fZ];
}

bool CGameTerrain::ValidPosition(int fX, int fZ)
{
	return true;
}

/* 
	Generate movable tile numbers according to the terrain height.
*/
template <class T>
T T_Abs(const T a) { return ((a > 0) ? a : -a); }
bool CGameTerrain::MakeMoveTable(short ** ppEvents)
{
	const int NOTMOVE_HEIGHT = 10;
	float fMax, fMin;
	for (auto x = 0; x < m_iHeightMapSize - 1; x++)
	{
		for (auto z = 0; z < m_iHeightMapSize - 1; z++)
		{
			fMax = -FLT_MAX;
			fMin = FLT_MAX;

			if (fMax < m_ppHeights[x][z]) fMax = m_ppHeights[x][z];
			if (fMin > m_ppHeights[x][z]) fMin = m_ppHeights[x][z];

			if (fMax < m_ppHeights[x][z + 1]) fMax = m_ppHeights[x][z + 1];
			if (fMin > m_ppHeights[x][z + 1]) fMin = m_ppHeights[x][z + 1];

			if (fMax < m_ppHeights[x + 1][z]) fMax = m_ppHeights[x + 1][z];
			if (fMin > m_ppHeights[x + 1][z]) fMin = m_ppHeights[x + 1][z];

			if (fMax < m_ppHeights[x + 1][z + 1]) fMax = m_ppHeights[x + 1][z + 1];
			if (fMin > m_ppHeights[x + 1][z + 1]) fMin = m_ppHeights[x + 1][z + 1];

			if (NOTMOVE_HEIGHT <= T_Abs(static_cast<int>(fMax - fMin)))
			{
				ppEvents[x][z] = 0;
			}
		}
	}
	return false;
}