/**
 * ______________________________________________________
 * This file is part of ko-smd-generator project.
 * 
 * @author       Mustafa Kemal Gılor <mustafagilor@gmail.com> (2016)
 * .
 * SPDX-License-Identifier:	MIT
 * ______________________________________________________
 */

#pragma once

#include "My_3DStruct.h"
#include "types.h"
#include <cstdio>
#include "DebugUtils.h"
#include "N3ShapeEx.h"
#include <list>
#include <vector>

const int CELL_MAIN_DEVIDE = 4;
const int CELL_SUB_SIZE = 4; // 4m
const int CELL_MAIN_SIZE = CELL_MAIN_DEVIDE * CELL_SUB_SIZE;
const int MAX_CELL_MAIN = 4096 / CELL_MAIN_SIZE;
const int MAX_CELL_SUB = MAX_CELL_MAIN * CELL_MAIN_DEVIDE;

#define WORD unsigned __int16



class CN3ShapeMgr final
{
public:
	struct __CellSub
	{
		int 	nCCPolyCount; // Collision Check Polygon Count
		uint32*	pdwCCVertIndices; // Collision Check Polygon Vertex Indices - wCCPolyCount * 3

		void Load(FILE *fp)
		{
			if (fread(&nCCPolyCount, sizeof(int), 1, fp) != 1)
			{
				//ASSERT(0);
				return;
			}

			if (nCCPolyCount != 0)
			{
				if (pdwCCVertIndices) 
					delete [] pdwCCVertIndices;

				pdwCCVertIndices = new uint32[nCCPolyCount * 3];
				if (fread(pdwCCVertIndices, nCCPolyCount * 3 * 4, 1, fp) != 1)
				{
					//ASSERT(0);
					return;
				}
			}
		}
		/*	DWORD dwRWC = 0;
			WriteFile(hFile, &nCCPolyCount, 4, &dwRWC, NULL);
			if(nCCPolyCount > 0)
				WriteFile(hFile, pdwCCVertIndices, nCCPolyCount * 3 * 4, &dwRWC, NULL);*/
		void Save(FILE *fp)
		{
			unsigned __int32 dwRWC = 0;
			fwrite(&nCCPolyCount, 4, 1, fp);
			if (nCCPolyCount > 0)
				fwrite(pdwCCVertIndices, 4, nCCPolyCount * 3, fp);
		}

		__CellSub() { memset(this, 0, sizeof(__CellSub)); }
		~__CellSub() { delete [] pdwCCVertIndices; }
	};

	struct __CellMain
	{
		int		nShapeCount; // Shape Count;
		WORD*	pwShapeIndices; // Shape Indices
		__CellSub SubCells[CELL_MAIN_DEVIDE][CELL_MAIN_DEVIDE];

		void Load(FILE *fp)
		{
			if (fread(&nShapeCount, sizeof(int), 1, fp) != 1)
			{
				//ASSERT(0);
				return;
			}

			if (nShapeCount != 0)
			{
				if (pwShapeIndices) 
					delete [] pwShapeIndices;

				pwShapeIndices = new WORD[nShapeCount];
				if (fread(pwShapeIndices, nShapeCount * 2, 1, fp) != 1)
				{
					//ASSERT(0);
					return;
				}
			}

			for (int z = 0; z < CELL_MAIN_DEVIDE; z++)
			{
				for (int x = 0; x < CELL_MAIN_DEVIDE; x++)
					SubCells[x][z].Load(fp);
			}
		}

		/*	DWORD dwRWC = 0;
			WriteFile(hFile, &nShapeCount, 4, &dwRWC, NULL);
			if(nShapeCount > 0) WriteFile(hFile, pwShapeIndices, nShapeCount * 2, &dwRWC, NULL);
			for(int z = 0; z < CELL_MAIN_DEVIDE; z++)
			{
				for(int x = 0; x < CELL_MAIN_DEVIDE; x++)
				{
					SubCells[x][z].Save(hFile);
				}
			}*/
		void Save(FILE *fp)
		{
			fwrite(&nShapeCount, 4, 1, fp);
			if (nShapeCount > 0)
				fwrite(pwShapeIndices, sizeof(short), nShapeCount, fp);
			for (int z = 0; z < CELL_MAIN_DEVIDE; z++)
				for (int x = 0; x < CELL_MAIN_DEVIDE; x++)
					SubCells[x][z].Save(fp);
		}

		__CellMain() { nShapeCount = 0; pwShapeIndices = nullptr; }
		~__CellMain() { delete [] pwShapeIndices; }
	};

	__Vector3* 				m_pvCollisions;

	std::vector<CN3ShapeEx*> & GetShapes() { return m_Shapes; }

protected:
	float					m_fMapWidth;
	float					m_fMapLength;
	int					m_nCollisionFaceCount;
	__CellMain*				m_pCells[MAX_CELL_MAIN][MAX_CELL_MAIN];

	std::vector<CN3ShapeEx*>	m_Shapes;			// 리스트로 안 만든 이유는... 배열이 훨씬 효율적이기 때문이다.
	std::list<CN3ShapeEx*>	m_ShapesToRender;	// Tick 을 호출하면 렌더링할 것만 추린다..
	std::list<CN3ShapeEx*>	m_ShapesHaveID;		// ID 를 갖고 있어 NPC 가 될수 있는 Shapes....

	//std::vector<_OBJECT_EVENT>

public:
	void SubCell(const __Vector3& vPos, __CellSub** ppSubCell);
	__CellSub* SubCell(float fX, float fZ)
	{
		int x = static_cast<int>(fX / CELL_MAIN_SIZE);
		int z = static_cast<int>(fZ / CELL_MAIN_SIZE);
		
		// _ASSERT(x >= 0 && x < MAX_CELL_MAIN && z >= 0 && z < MAX_CELL_MAIN);
		if(nullptr == m_pCells[x][z]) return nullptr;

		int xx = (static_cast<int>(fX)%CELL_MAIN_SIZE)/CELL_SUB_SIZE;
		int zz = (static_cast<int>(fZ)%CELL_MAIN_SIZE)/CELL_SUB_SIZE;
		
		return &(m_pCells[x][z]->SubCells[xx][zz]);
	}
	float		GetHeightNearstPos(const __Vector3& vPos, __Vector3* pvNormal = nullptr);
	float		GetHeight(float fX, float fZ, __Vector3* pvNormal = nullptr);
	int			SubCellPathThru(const __Vector3& vFrom, const __Vector3& vAt, __CellSub** ppSubCells);
	float		Width() { return m_fMapWidth; }
	float		Height() { return m_fMapWidth; }
	int ColFaceCount() const { return m_nCollisionFaceCount; }

	bool CheckCollision(const __Vector3& vInitialPos, const __Vector3& vNewPos, const __Vector3 &vDirection);
	bool		CheckCollision(	const __Vector3& vPos,
								const __Vector3& vDir,
								float fSpeedPerSec,
								__Vector3* pvCol = nullptr,
								__Vector3* pvNormal = nullptr,
								__Vector3* pVec = nullptr);

	bool		Create(float fMapWidth, float fMapLength);
	bool		LoadCollisionData(FILE *fp);
	bool		SaveCollisionData(FILE  *fp);

	void MakeMoveTable(short ** ppEvent);

	//void GenerateObjectEventList();

	bool Load(FILE * fp,bool oldMode = false);
	void		ReleaseShapes();
	void DoCleanup();
	void Release();
	CN3ShapeMgr();
	virtual ~CN3ShapeMgr();
};
