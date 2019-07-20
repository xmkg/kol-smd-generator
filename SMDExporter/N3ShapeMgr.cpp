#include "stdafx.h"
#include "N3ShapeMgr.h"
#include "N3ShapeEx.h"

const uint32 OBJ_SHAPE_EXTRA = 0x1000;


CN3ShapeMgr::CN3ShapeMgr()
{
	m_fMapWidth = 0.0f;
	m_fMapLength = 0.0f;
	m_nCollisionFaceCount = 0;
	m_pvCollisions = nullptr;
	memset(m_pCells, 0, sizeof(m_pCells));
}

CN3ShapeMgr::~CN3ShapeMgr()
{
	if (m_pvCollisions != nullptr)
	{
		delete [] m_pvCollisions; 
		m_pvCollisions = nullptr;
	}

	for(int z = 0; z < MAX_CELL_MAIN; z++)
	{
		for(int x = 0; x < MAX_CELL_MAIN; x++)
		{
			delete m_pCells[x][z];
		}
	}
}

void CN3ShapeMgr::Release()
{
	m_fMapWidth = 0.0f;
	m_fMapLength = 0.0f;
	m_nCollisionFaceCount = 0;

	if (m_pvCollisions != nullptr)
	{
		delete [] m_pvCollisions; 
		m_pvCollisions = nullptr;
	}

	for(int z = 0; z < MAX_CELL_MAIN; z++)
	{
		for(int x = 0; x < MAX_CELL_MAIN; x++)
		{
			delete m_pCells[x][z];
			m_pCells[x][z] = nullptr;
		}
	}
	memset(m_pCells, 0, sizeof(MAX_CELL_MAIN));
}

bool CN3ShapeMgr::LoadCollisionData(FILE *fp)
{
	if (fread(&m_fMapWidth, 4, 1, fp) != 1
		|| fread(&m_fMapLength, 4, 1, fp) != 1)
	{
		ASSERT(0);
		return false;
	}

	Create(m_fMapWidth, m_fMapLength);

	if (fread(&m_nCollisionFaceCount, 4, 1, fp) != 1)
	{
		ASSERT(0);
		return false;
	}

	if (m_pvCollisions != nullptr)
	{
		delete [] m_pvCollisions; 
		m_pvCollisions = nullptr;
	}

	if (m_nCollisionFaceCount > 0)
	{
		m_pvCollisions = new __Vector3[m_nCollisionFaceCount * 3];

		auto readSize = sizeof(__Vector3) * m_nCollisionFaceCount * 3;

		size_t readCnt = fread(m_pvCollisions, readSize, 1, fp);
		//printf("readcnt : %d, readsize : %d\n", readCnt, readSize);
		if (readCnt!= 1)
		{
			ASSERT(0);
			return false;
		}
	}

	// Cell data
	int z = 0;
	for(float fZ = 0.0f; fZ < m_fMapLength; fZ += CELL_MAIN_SIZE, z++)
	{
		int x = 0;
		for(float fX = 0.0f; fX < m_fMapWidth;  fX += CELL_MAIN_SIZE, x++)
		{
			if (m_pCells[x][z] != nullptr)
			{
				delete m_pCells[x][z]; 
				m_pCells[x][z] = nullptr;
			}

			uint32 bExist;
			if (fread(&bExist, 4, 1, fp) != 1)
			{
				ASSERT(0);
				return false;
			}

			if (!bExist) 
				continue;

			m_pCells[x][z] = new __CellMain;
			m_pCells[x][z]->Load(fp);
		}
	}

	return true;
}

bool CN3ShapeMgr::SaveCollisionData(FILE * fp)
{
//	unsigned __int32 dwRWC;

	fwrite(&m_fMapWidth, 4, 1, fp); // 맵 실제 미터 단위 너비
	fwrite(&m_fMapLength, 4, 1, fp);
	//WriteFile(hFile, &m_fMapLength, 4, &dwRWC, NULL); // 맵 실제 미터 단위 길이

													  // 충돌 체크 폴리곤 데이터 쓰기..
//	WriteFile(hFile, &m_nCollisionFaceCount, 4, &dwRWC, NULL);
	fwrite(&m_nCollisionFaceCount, 4, 1, fp);
	//auto writeSize = sizeof(__Vector3) * m_nCollisionFaceCount * 3;
//	static const int vSize = sizeof(__Vector3);
	if (m_nCollisionFaceCount > 0)
	{
	/*	for (int i = 0; i < m_nCollisionFaceCount * 3; i++)
			fwrite(&m_pvCollisions[i], vSize, 1, fp);*/
		if (fwrite(m_pvCollisions, sizeof(__Vector3), m_nCollisionFaceCount * 3, fp) != m_nCollisionFaceCount * 3)
			ASSERT(0);
		//WriteFile(hFile, m_pvCollisions, sizeof(__Vector3) * m_nCollisionFaceCount * 3, &dwRWC, NULL);
	}
//	return true;
	// Cell Data 쓰기.
	int z = 0;
	for (float fZ = 0.0f; fZ < m_fMapLength; fZ += CELL_MAIN_SIZE, z++)
	{
		int x = 0;
		for (float fX = 0.0f; fX < m_fMapWidth; fX += CELL_MAIN_SIZE, x++)
		{
			unsigned __int32 bExist = false;
			if (m_pCells[x][z]) bExist = true;
			fwrite(&bExist, 4, 1, fp);
			//WriteFile(hFile, &bExist, 4, &dwRWC, NULL); // 데이터가 있는 셀인지 쓰고..

			if (NULL == m_pCells[x][z]) continue;

			m_pCells[x][z]->Save(fp);
		}
	}

	return true;
}

void CN3ShapeMgr::MakeMoveTable(short** pMoveArray)
{
	int ArraySize = (MAX_CELL_MAIN * CELL_MAIN_DEVIDE) + 1;

	for (int bx = 0; bx<MAX_CELL_MAIN; bx++)
	{
		for (int bz = 0; bz<MAX_CELL_MAIN; bz++)
		{
			if (m_pCells[bx][bz])
			{
				for (int sx = 0; sx<CELL_MAIN_DEVIDE; sx++)
				{
					for (int sz = 0; sz<CELL_MAIN_DEVIDE; sz++)
					{
						if (m_pCells[bx][bz]->SubCells[sx][sz].nCCPolyCount>0)
						{
							int ix, iz;
							ix = (bx * CELL_MAIN_DEVIDE) + sx;
							iz = (bz * CELL_MAIN_DEVIDE) + sz;
							pMoveArray[ix][iz] = 0;
						}
					} //for(int sz=0; sz<CELL_MAIN_DEVIDE; sz++)
				} //for(int sx=0; sx<CELL_MAIN_DEVIDE; sx++)
			} //if(m_pCells[bx][bz] && m_pCells[bx][bz]->nShapeCount>0)
		} //for(int bz=0; bz<MAX_CELL_MAIN; bz++)
	} //for(int bx=0; bx<MAX_CELL_MAIN; bx++)
}

/*char  temp(int a4)
{
	auto v9 = a4;
	auto v10 = 5640;
	if (!a4)
		return 1;
	v11 = a3;
	do
	{
		LOWORD(a1) = *(_BYTE *)v11;
		*(unsigned __int8 *)(v11 + a5 - a3) = *(_BYTE *)v11 ^ BYTE1(v10);
		++v11;
		--v9;
		a1 = 1035 * (v10 + a1);
		v10 = 2 * a1 + 24705;
	} while (v9);
}*/

std::string DecryptStringX(const std::string & encStr)
{
	const unsigned short CipherKey1 = 0x0816;
//	const unsigned short CipherKey2 = 0x1608;
	char buffer[512] = "";


	unsigned short _volatileKey = 0x6081;
	for (int i = 0; i < encStr.length() - 1; i++)
	{
		unsigned __int8 rawByte = encStr[i];
		unsigned __int8 temporaryKey = (unsigned __int8)((_volatileKey & 0xff00) >> 8);
		unsigned __int8 encryptedByte = (unsigned __int8)(temporaryKey ^ rawByte);
		_volatileKey = (unsigned short)((rawByte + _volatileKey)*CipherKey1 /*+ CipherKey2*/);
		buffer[i] = encryptedByte;
	}
	// null terminator

	return std::string(buffer);
}

bool CN3ShapeMgr::Load(FILE* fp,bool oldMode)
{

	/* Load collision data */
	if (false == LoadCollisionData(fp)) 
		return false;

	int nShapeCount = 0;
	if (!m_Shapes.empty()) 
		ReleaseShapes();
	m_ShapesHaveID.clear();

	// this is unknown..
	//fread_s(&nUnknown, 4, 4, 1, fp);

	fread_s(&nShapeCount, 4, 4, 1, fp);

//	ReadFile(hFile, &iSC, 4, &dwRWC, NULL); // Shape Count
	if (nShapeCount > 0)
	{
		CN3ShapeEx* pShape = nullptr;
		m_Shapes.reserve(nShapeCount);
		uint32 dwType = 0;

		int32 halfWay = nShapeCount / 2; // Why the fuck did they do that..
		for (int i = 0; i < nShapeCount; i++)
		{
			/// Read object type
			fread_s(&dwType, 4, 4, 1, fp);
			//ReadFile(hFile, &dwType, 4, &dwRWC, NULL); // Shape Type
			if (dwType & OBJ_SHAPE_EXTRA) 
				pShape =new CN3ShapeEx(); // 성문등 확장된 Object 로 쓸경우..
			else pShape =new CN3ShapeEx();
			//pShape = new CN3ShapeEx();
			

			// pShape->m_iEventID; 바인드 포인트 100~, 200~ 성문 1100~, 1200~ 레버 2100~, 2200~
			// pShape->m_iEventType; 0-바인드포인트 1-성문(좌우열림) 2-성문(상하열림) 3-레버(상하당김) 4-깃발(보임, 안보임)
			// pShape->m_iNPC_ID; 조종할 Object ID
			// pShape->m_iNPC_Status; toggle 0, 1
		
			pShape->Load(fp);

			#ifdef GSMD_TOOL

			TRACE("CN3ShapeMgr::Load() - Object report (# %d of %d), Offset : %llu", i+1, nShapeCount, pShape->GetOffset());
			TRACE(" - Object name %s, dwType : %d, exFlag : %s", pShape->GetName().c_str(),dwType,dwType & OBJ_SHAPE_EXTRA ? "true" : "false");
			TRACE(" - Object location [x(%g),y(%g),z(%g)]", pShape->GetVPosition().x, pShape->GetVPosition().y, pShape->GetVPosition().z);
			TRACE(" - Object scale -  [x(%g),y(%g),z(%g)]", pShape->GetVScale().x, pShape->GetVScale().y, pShape->GetVScale().z);
			TRACE(" - Object event id : %d, event type %d", pShape->GetEventID(), pShape->GetEventType());
			TRACE(" - Object belong id : %d, npc id %d, status %d", pShape->GetBelongID(), pShape->GetNpcID(), pShape->GetNpcStatus());
			PartMap & parts = pShape->GetPartMap();
			TRACE(" - Object parts (count %d)", parts.size());

			enum ObjectType
			{
				OBJECT_BIND = 0,
				OBJECT_GATE = 1,
				OBJECT_GATE2 = 2,
				OBJECT_GATE_LEVER = 3,
				OBJECT_FLAG_LEVER = 4,
				OBJECT_WARP_GATE = 5,
				OBJECT_BARRICADE = 6,
				OBJECT_REMOVE_BIND = 7,
				OBJECT_ANVIL = 8,
				OBJECT_ARTIFACT = 9,
				OBJECT_NPC = 11
			};

			if (pShape->GetEventID() > 0)
			{
				std::string eventType;
				switch (pShape->GetEventType())
				{
				case OBJECT_BIND:
					eventType = "Bind point";
					break;
				case OBJECT_GATE:
					eventType = "Gate Type 1 (left-right)";
					break;
				case OBJECT_GATE2:
					eventType = "Gate Type 2 (up-down)";
					break;
				case OBJECT_GATE_LEVER:
					eventType = "Gate Lever";
					break;
				case OBJECT_FLAG_LEVER:
					eventType = "Flag";
					break;
				case OBJECT_WARP_GATE:
					eventType = "Warp Gate";
					break;
				case OBJECT_BARRICADE:
					eventType = "Barricade";
					break;
				case OBJECT_REMOVE_BIND:
					eventType = "Resurrection Point";
					break;
				case OBJECT_ANVIL:
					eventType = "Magic Anvil";
					break;
				case OBJECT_ARTIFACT:
					eventType = "Artifact";
					break;
				case OBJECT_NPC:
					eventType = "NPC";
					break;
				default:
					eventType = "unknown type";
					break;
				}
				printf("- Object (%s) #%d is event object\n-- Type : %s(%d), Position(%g,%g,%g)\n", pShape->GetName().c_str(), i + 1, eventType.c_str(), pShape->GetEventType(), pShape->GetVPosition().x, pShape->GetVPosition().y, pShape->GetVPosition().z);
			}
		/*	if (pShape->GetEventType() != 0)
				printf("object #%d event object\n", i + 1);*/

			for(const auto & part:parts)
			{
				TRACE(" -- Part mesh %s", part.first.c_str());
				TRACE(" -- Part textures (count %d)", part.second.size());
				for(const auto & texture:part.second)
				{
					TRACE(" --- %s", texture.c_str());
				}
			}
			#endif
			// New file format has an idiotic thing ..
			if(!oldMode && halfWay-1 == i)
			{
				printf("ShapeData halfway offset (mode new) : %d\n", ftell(fp));
				uint32 strmfLen;
				fread_s(&strmfLen, 4, 4, 1, fp);
				std::vector<char> buffer(strmfLen + 1, NULL);
				fread_s(&(buffer[0]), strmfLen, 1, strmfLen, fp);
				std::string mapNameHalfway(buffer.begin(), buffer.end());

				/*auto ikey = 0x0816;
				for (auto i = 0; i < strmfLen; i++)
				{
					buffer[i] ^= 0x0816;
					ikey = 0x1608;
				}
				*/
			/*	std::string zx = DecryptStringX(mapNameHalfway);
				__debugbreak();*/
			}
			else if(oldMode && halfWay - 1 == i)
			{
				printf("ShapeData halfway offset (mode old) : %d\n", ftell(fp));
			}

			m_Shapes.push_back(pShape);
		
		}
	}

	return true;

}

void CN3ShapeMgr::ReleaseShapes()
{
	int iSC = m_Shapes.size();
	for (int i = 0; i < iSC; i++) delete m_Shapes[i];
	m_Shapes.clear();
	m_ShapesHaveID.clear();
	m_ShapesToRender.clear();
}

void CN3ShapeMgr::DoCleanup()
{
	ReleaseShapes();
}

bool CN3ShapeMgr::Create(float fMapWidth, float fMapLength) // ¸ÊÀÇ ³Êºñ¿Í ³ôÀÌ¸¦ ¹ÌÅÍ ´ÜÀ§·Î ³Ö´Â´Ù..
{
	if(	fMapWidth <= 0.0f || fMapWidth > MAX_CELL_MAIN * CELL_MAIN_SIZE ||
		fMapLength <= 0.0f || fMapLength > MAX_CELL_MAIN * CELL_MAIN_SIZE )
	{
		return false;
	}

	m_fMapWidth = fMapWidth;
	m_fMapLength = fMapLength;

	return true;
}

bool CN3ShapeMgr::CheckCollision(const __Vector3& vInitialPos, const __Vector3& vNewPos, const __Vector3& vDirection)
{
	static __CellSub* ppCells[128];

	int nSubCellCount = this->SubCellPathThru(vInitialPos, vNewPos, ppCells); // Åë°úÇÏ´Â ¼­ºê¼¿À» °¡Á®¿Â´Ù..
	if (nSubCellCount <= 0 || nSubCellCount > 128)
		return false; // ¾øÀ½ ¸»ÀÚ.

	__Vector3 vColTmp(0, 0, 0);
	int nIndex0, nIndex1, nIndex2;
	static float fT, fU, fV, fDistTmp, fDistClosest;
	fDistClosest = FLT_MAX;

	fDistClosest = FLT_MAX;

	for (int i = 0; i < nSubCellCount; i++)
	{
		if (ppCells[i]->nCCPolyCount <= 0) continue;

		for (int j = 0; j < ppCells[i]->nCCPolyCount; j++)
		{
			nIndex0 = ppCells[i]->pdwCCVertIndices[j * 3];
			nIndex1 = ppCells[i]->pdwCCVertIndices[j * 3 + 1];
			nIndex2 = ppCells[i]->pdwCCVertIndices[j * 3 + 2];

			if (false == ::_IntersectTriangle(vInitialPos, vDirection, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2], fT, fU, fV, &vColTmp)) continue;
			if (false == ::_IntersectTriangle(vNewPos, vDirection, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2]))
			{
				fDistTmp = (vInitialPos - vColTmp).Magnitude(); // °Å¸®¸¦ Àçº¸°í..
				if (fDistTmp < fDistClosest)
				{
					fDistClosest = fDistTmp;
				}
			}
		}
	}

	if (fDistClosest != FLT_MAX)
		return true;

	return false;
}

bool CN3ShapeMgr::CheckCollision(	const __Vector3& vPos,		// Ãæµ¹ À§Ä¡
									const __Vector3& vDir,		// ¹æÇâ º¤ÅÍ
									float fSpeedPerSec,			// ÃÊ´ç ¿òÁ÷ÀÌ´Â ¼Óµµ
									__Vector3* pvCol,			// Ãæµ¹ ÁöÁ¡
									__Vector3* pvNormal,		// Ãæµ¹ÇÑ¸éÀÇ ¹ı¼±º¤ÅÍ
									__Vector3* pVec)			// Ãæµ¹ÇÑ ¸é ÀÇ Æú¸®°ï __Vector3[3]
{
	if(fSpeedPerSec <= 0) return false; // ¿òÁ÷ÀÌ´Â ¼Óµµ°¡ ¾ø°Å³ª ¹İ´ë·Î ¿òÁ÷ÀÌ¸é ³Ñ¾î°£´Ù..
	static __CellSub* ppCells[128];
	//__Vector3 vPosNext = vPos;
	__Vector3 vPosNext = vPos + (vDir* fSpeedPerSec); // ´ÙÀ½ À§Ä¡
	int nSubCellCount = this->SubCellPathThru(vPos, vPosNext, ppCells); // Åë°úÇÏ´Â ¼­ºê¼¿À» °¡Á®¿Â´Ù..
	if(nSubCellCount <= 0 || nSubCellCount > 128) return false; // ¾øÀ½ ¸»ÀÚ.

	__Vector3 vColTmp(0,0,0);
	int nIndex0, nIndex1, nIndex2;
	static float fT, fU, fV, fDistTmp, fDistClosest;
	fDistClosest = FLT_MAX;

	for ( int i = 0; i < nSubCellCount; i++ )
	{
		if ( ppCells[i]->nCCPolyCount <= 0 ) continue;

		for ( int j = 0; j < ppCells[i]->nCCPolyCount; j++ )
		{
			nIndex0 = ppCells[i]->pdwCCVertIndices[j*3];
			nIndex1 = ppCells[i]->pdwCCVertIndices[j*3+1];
			nIndex2 = ppCells[i]->pdwCCVertIndices[j*3+2];
			
			if(false == ::_IntersectTriangle(vPos, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2], fT, fU, fV, &vColTmp)) continue;
			if(false == ::_IntersectTriangle(vPosNext, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2]))
			{
				fDistTmp = (vPos - vColTmp).Magnitude(); // °Å¸®¸¦ Àçº¸°í..
				if(fDistTmp < fDistClosest) 
				{
					fDistClosest = fDistTmp;
					// Ãæµ¹ÀÌ´Ù..
					if(pvCol) *pvCol = vColTmp;
					if(pvNormal)
					{
						(*pvNormal).Cross(m_pvCollisions[nIndex1] - m_pvCollisions[nIndex0], 
							m_pvCollisions[nIndex2] - m_pvCollisions[nIndex0]);
						(*pvNormal).Normalize();
					}
					if ( pVec )
					{
						pVec[0] = m_pvCollisions[nIndex0];
						pVec[1] = m_pvCollisions[nIndex1];
						pVec[2] = m_pvCollisions[nIndex2];
					}
				}
			}
		}
	}
	
	if(fDistClosest != FLT_MAX)
		return true;

	return false;
}

int CN3ShapeMgr::SubCellPathThru(const __Vector3& vFrom, const __Vector3& vAt, __CellSub** ppSubCells) // º¤ÅÍ »çÀÌ¿¡ °ÉÄ£ ¼¿Æ÷ÀÎÅÍ µ¹·ÁÁØ´Ù..
{
	if(nullptr == ppSubCells) return 0;

	// ¹üÀ§¸¦ Á¤ÇÏ°í..
	int xx1 = 0, xx2 = 0, zz1 = 0, zz2 = 0;

	if(vFrom.x < vAt.x) { xx1 = (int)(vFrom.x / CELL_SUB_SIZE); xx2 = (int)(vAt.x / CELL_SUB_SIZE); }
	else { xx1 = (int)(vAt.x / CELL_SUB_SIZE); 	xx2 = (int)(vFrom.x / CELL_SUB_SIZE); }

	if(vFrom.z < vAt.z)	{ zz1 = (int)(vFrom.z / CELL_SUB_SIZE); zz2 = (int)(vAt.z / CELL_SUB_SIZE); }
	else { zz1 = (int)(vAt.z / CELL_SUB_SIZE); zz2 = (int)(vFrom.z / CELL_SUB_SIZE); }

	bool bPathThru;
	float fZMin, fZMax, fXMin, fXMax;
	int nSubCellCount = 0;
	for(int z = zz1; z <= zz2; z++) // ¹üÀ§¸¸Å­ Ã³¸®..
	{
		fZMin = (float)(z * CELL_SUB_SIZE);
		fZMax = (float)((z+1) * CELL_SUB_SIZE);
		for(int x = xx1; x <= xx2; x++)
		{
			fXMin = (float)(x * CELL_SUB_SIZE);
			fXMax = (float)((x+1) * CELL_SUB_SIZE);

			// Cohen thuderland algorythm
			uint32 dwOC0 = 0, dwOC1 = 0; // OutCode 0, 1
			if(vFrom.z > fZMax) dwOC0 |= 0xf000;
			if(vFrom.z < fZMin) dwOC0 |= 0x0f00;
			if(vFrom.x > fXMax) dwOC0 |= 0x00f0;
			if(vFrom.x < fXMin) dwOC0 |= 0x000f;
			if(vAt.z > fZMax) dwOC1 |= 0xf000;
			if(vAt.z < fZMin) dwOC1 |= 0x0f00;
			if(vAt.x > fXMax) dwOC1 |= 0x00f0;
			if(vAt.x < fXMin) dwOC1 |= 0x000f;
			
			bPathThru = false;
			if(dwOC0 & dwOC1) bPathThru = false; // µÎ ³¡Á¡ÀÌ °°Àº º¯ÀÇ ¿ÜºÎ¿¡ ÀÖ´Ù.
			else if(dwOC0 == 0 && dwOC1 == 0) bPathThru = true;// ¼±ºĞÀÌ »ç°¢Çü ³»ºÎ¿¡ ÀÖÀ½
			else if((dwOC0 == 0 && dwOC1 != 0) || (dwOC0 != 0 && dwOC1 == 0)) bPathThru = true;// ¼±ºĞ ÇÑÁ¡Àº ¼¿ÀÇ ³»ºÎ¿¡ ÇÑÁ¡Àº ¿ÜºÎ¿¡ ÀÖÀ½.
			else if((dwOC0 & dwOC1) == 0) // µÎ …LÁ¡ ¸ğµÎ ¼¿ ¿ÜºÎ¿¡ ÀÖÁö¸¸ ÆÇ´ÜÀ» ´Ù½Ã ÇØ¾ß ÇÑ´Ù.
			{
				float fXCross = vFrom.x + (fZMax - vFrom.z) * (vAt.x - vFrom.x) / (vAt.z - vFrom.z); // À§ÀÇ º¯°úÀÇ ±³Â÷Á¡À» °è»êÇÏ°í..
				if(fXCross < fXMin) bPathThru = false; // ¿ÏÀüÈ÷ ¿Ü°û¿¡ ÀÖ´Ù.
				else bPathThru = true; // °ÉÃ³ÀÖ´Ù.
			}

			if(false == bPathThru) continue;

			// Ãæµ¹ Á¤º¸¸¦ ½á¾ß ÇÑ´Ù..
			int nX = x / CELL_MAIN_DEVIDE;
			int nZ = z / CELL_MAIN_DEVIDE;
			if(nX < 0 || nX >= MAX_CELL_MAIN || nZ < 0 && nZ >= MAX_CELL_MAIN) continue; // ¸ŞÀÎ¼¿¹Ù±ù¿¡ ÀÖÀ½ Áö³ª°£´Ù.
			if(nullptr == m_pCells[nX][nZ]) continue; // ¸ŞÀÎ¼¿ÀÌ ³ÎÀÌ¸é Áö³ª°£´Ù..

			int nXSub = x%CELL_MAIN_DEVIDE;
			int nZSub = z%CELL_MAIN_DEVIDE;

			ppSubCells[nSubCellCount] = &(m_pCells[nX][nZ]->SubCells[nXSub][nZSub]);
			nSubCellCount++;
		} // end of for(int x = xx1; x <= xx2; x++)
	} // end of for(int z = zz1; z <= zz2; z++) // ¹üÀ§¸¸Å­ Ã³¸®..

	return nSubCellCount; // °ÉÄ£ ¼¿ Æ÷ÀÎÅÍ µ¹·ÁÁÖ±â..
}

float CN3ShapeMgr::GetHeightNearstPos(const __Vector3 &vPos, __Vector3* pvNormal) // °¡Àå °¡±î¿î ³ôÀÌ°ªÀ» µ¹·ÁÁØ´Ù. ¾øÀ¸¸é -FLT_MAX À» µ¹·ÁÁØ´Ù.
{
	__CellSub* pCell = this->SubCell(vPos.x, vPos.z); // ¼­ºê¼¿À» °¡Á®¿Â´Ù..
	if(nullptr == pCell || pCell->nCCPolyCount <= 0) return -FLT_MAX; // ¾øÀ½ ¸»ÀÚ.

	__Vector3 vPosV = vPos; vPosV.y = 5000.0f; // ²À´ë±â¿¡ À§Ä¡¸¦ ÇÏ°í..
	__Vector3 vDir(0,-1, 0); // ¼öÁ÷ ¹æÇâ º¤ÅÍ
	__Vector3 vColTmp(0,0,0); // ÃÖÁ¾ÀûÀ¸·Î °¡Àå °¡±î¿î Ãæµ¹ À§Ä¡..

	int nIndex0, nIndex1, nIndex2;
	float fT, fU, fV;
	float fNearst = FLT_MAX, fMinTmp = 0, fHeight = -FLT_MAX;		// ÀÏ´Ü ÃÖ¼Ò°ªÀ» Å«°ªÀ¸·Î Àâ°í..

	for ( int i = 0; i < pCell->nCCPolyCount; i++ )
	{
		nIndex0 = pCell->pdwCCVertIndices[i*3];
		nIndex1 = pCell->pdwCCVertIndices[i*3+1];
		nIndex2 = pCell->pdwCCVertIndices[i*3+2];
		
		// Ãæµ¹µÈ Á¡ÀÌ ÀÖÀ¸¸é..
		if(true == ::_IntersectTriangle(vPosV, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2], fT, fU, fV, &vColTmp))
		{
			fMinTmp = (vColTmp - vPos).Magnitude();
			if(fMinTmp < fNearst) // °¡Àå °¡±î¿î Ãæµ¹ À§Ä¡¸¦ Ã£±â À§ÇÑ ÄÚµå..
			{
				fNearst = fMinTmp;
				fHeight = vColTmp.y; // ³ôÀÌ°ª.

				if(pvNormal)
				{
					pvNormal->Cross(m_pvCollisions[nIndex1] - m_pvCollisions[nIndex0], m_pvCollisions[nIndex2] - m_pvCollisions[nIndex0]);
					pvNormal->Normalize();
				}
			}
		}
	}

	return fHeight;
}

float CN3ShapeMgr::GetHeight(float fX, float fZ, __Vector3* pvNormal) // °¡Àå ³ôÀº °÷À» µ¹·ÁÁØ´Ù.. ¾øÀ¸¸é -FLT_MAX°ªÀ» µ¹·ÁÁØ´Ù.
{
	__CellSub* pCell = this->SubCell(fX, fZ); // ¼­ºê¼¿À» °¡Á®¿Â´Ù..
	if(nullptr == pCell || pCell->nCCPolyCount <= 0) return -FLT_MAX; // ¾øÀ½ ¸»ÀÚ.

	__Vector3 vPosV(fX, 5000.0f, fZ); // ²À´ë±â¿¡ À§Ä¡¸¦ ÇÏ°í..
	__Vector3 vDir(0,-1, 0); // ¼öÁ÷ ¹æÇâ º¤ÅÍ
	__Vector3 vColTmp(0,0,0); // ÃÖÁ¾ÀûÀ¸·Î °¡Àå °¡±î¿î Ãæµ¹ À§Ä¡..

	int nIndex0, nIndex1, nIndex2;
	float fT, fU, fV;
	float fMaxTmp = -FLT_MAX;;

	for ( int i = 0; i < pCell->nCCPolyCount; i++ )
	{
		nIndex0 = pCell->pdwCCVertIndices[i*3];
		nIndex1 = pCell->pdwCCVertIndices[i*3+1];
		nIndex2 = pCell->pdwCCVertIndices[i*3+2];
		
		// Ãæµ¹µÈ Á¡ÀÌ ÀÖÀ¸¸é..
		if(true == ::_IntersectTriangle(vPosV, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2], fT, fU, fV, &vColTmp))
		{
			if(vColTmp.y > fMaxTmp)
			{
				fMaxTmp = vColTmp.y;
				if(pvNormal)
				{
					pvNormal->Cross(m_pvCollisions[nIndex1] - m_pvCollisions[nIndex0], m_pvCollisions[nIndex2] - m_pvCollisions[nIndex0]);
					pvNormal->Normalize();
				}
			}
		}
	}

	return fMaxTmp;
}

void CN3ShapeMgr::SubCell(const __Vector3& vPos, __CellSub** ppSubCell)			// ÇØ´ç À§Ä¡ÀÇ ¼¿ Æ÷ÀÎÅÍ¸¦ µ¹·ÁÁØ´Ù.
{
	int x = (int)(vPos.x / CELL_MAIN_SIZE);
	int z = (int)(vPos.z / CELL_MAIN_SIZE);
	
	// _ASSERT(x >= 0 && x < MAX_CELL_MAIN && z >= 0 && z < MAX_CELL_MAIN);

	int xx = (((int)vPos.x)%CELL_MAIN_SIZE)/CELL_SUB_SIZE;			// 2, 3, 4
	int zz = (((int)vPos.z)%CELL_MAIN_SIZE)/CELL_SUB_SIZE;			// 1, 0, 5
																	// 8, 7, 6	
	for ( int i = 0; i < 9; i++ )
	{
		switch( i )
		{
			case 0:
				if ( m_pCells[x][z] != nullptr )
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx][zz]);
				else
					ppSubCell[i] = nullptr;
				break;

			case 1:
				if ( (x == 0) && (xx == 0) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (x != 0) && (xx == 0) )
				{
					if ( m_pCells[x-1][z] != nullptr )
						ppSubCell[i] = &(m_pCells[x-1][z]->SubCells[CELL_MAIN_DEVIDE-1][zz]);
					else
						ppSubCell[i] = nullptr;
					break;
				}

				if ( m_pCells[x][z] != nullptr )
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx-1][zz]);
				else
					ppSubCell[i] = nullptr;
				break;

			case 2:
				if ( (x == 0) && (xx == 0) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (z == (CELL_MAIN_SIZE-1)) && ( zz == (CELL_MAIN_DEVIDE-1) ) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (x != 0) && (xx == 0) )											// x °¨¼Ò, z Áõ°¡.
				{
					if ( (z != (MAX_CELL_MAIN-1)) && ( zz == (CELL_MAIN_DEVIDE-1) ) )
						if ( m_pCells[x-1][z+1] != nullptr )
							ppSubCell[i] = &(m_pCells[x-1][z+1]->SubCells[CELL_MAIN_DEVIDE-1][0]);
						else
							ppSubCell[i] = nullptr;
					else
						if ( m_pCells[x-1][z] != nullptr )
							ppSubCell[i] = &(m_pCells[x-1][z]->SubCells[CELL_MAIN_DEVIDE-1][zz+1]);
						else
							ppSubCell[i] = nullptr;
					break;
				}

				if ( (z != (MAX_CELL_MAIN-1)) && (zz == (CELL_MAIN_DEVIDE-1) ) )		// x °¨¼Ò, z Áõ°¡.
				{
					if ( (x != 0) && (xx == 0) )
						if ( m_pCells[x-1][z+1] != nullptr )
							ppSubCell[i] = &(m_pCells[x-1][z+1]->SubCells[CELL_MAIN_DEVIDE-1][0]);	
						else
							ppSubCell[i] = nullptr;
					else
						if ( m_pCells[x][z+1] != nullptr )
							ppSubCell[i] = &(m_pCells[x][z+1]->SubCells[xx-1][0]);	
						else
							ppSubCell[i] = nullptr;
					break;
				}
							
				if ( m_pCells[x][z] != nullptr )
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx-1][zz+1]);						
				else
					ppSubCell[i] = nullptr;					
					break;

			case 3:
				if ( (z == (MAX_CELL_MAIN-1)) && (zz == (CELL_MAIN_DEVIDE-1)) )			// z Áõ°¡.
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (z != (MAX_CELL_MAIN-1)) && (zz == (CELL_MAIN_DEVIDE-1)) )
				{
					if ( m_pCells[x-1][z] != nullptr )
						ppSubCell[i] = &(m_pCells[x-1][z]->SubCells[xx][0]);
					else
						ppSubCell[i] = nullptr;
					break;
				}

				if ( m_pCells[x][z] != nullptr )
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx][zz+1]);
				else
					ppSubCell[i] = nullptr;					
				break;

			case 4:
				if ( (x == (MAX_CELL_MAIN-1)) && (xx == (CELL_MAIN_DEVIDE-1)) )			// x Áõ°¡, z Áõ°¡.
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (z == (MAX_CELL_MAIN-1)) && ( zz == (CELL_MAIN_DEVIDE-1)) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (x != (MAX_CELL_MAIN-1)) && (xx == (CELL_MAIN_DEVIDE-1)) )
				{
					if ( (z != (MAX_CELL_MAIN-1)) && ( zz == (CELL_MAIN_DEVIDE-1)) )
						if ( m_pCells[x+1][z+1] != nullptr )
							ppSubCell[i] = &(m_pCells[x+1][z+1]->SubCells[0][0]);
						else
							ppSubCell[i] = nullptr;
					else
						if ( m_pCells[x+1][z] != nullptr )
							ppSubCell[i] = &(m_pCells[x+1][z]->SubCells[0][zz+1]);
						else
							ppSubCell[i] = nullptr;
					break;
				}

				if ( (z != (MAX_CELL_MAIN-1)) && (zz == (CELL_MAIN_DEVIDE-1)) )
				{
					if ( (x != (MAX_CELL_MAIN-1)) && (xx == (CELL_MAIN_DEVIDE-1)) )
						if ( m_pCells[x+1][z+1] != nullptr )
							ppSubCell[i] = &(m_pCells[x+1][z+1]->SubCells[0][0]);	
						else
							ppSubCell[i] = nullptr;
					else
						if ( m_pCells[x][z+1] != nullptr )
							ppSubCell[i] = &(m_pCells[x][z+1]->SubCells[xx+1][0]);	
						else
							ppSubCell[i] = nullptr;
					break;
				}

				if ( m_pCells[x][z] != nullptr )								
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx+1][zz+1]);						
				else
					ppSubCell[i] = nullptr;					
				break;

			case 5:																		// x Áõ°¡.
				if ( (x == (MAX_CELL_MAIN-1)) && (xx == (CELL_MAIN_DEVIDE-1)) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (x != (MAX_CELL_MAIN-1)) && (xx == (CELL_MAIN_DEVIDE-1)) )
				{
					if ( m_pCells[x+1][z] != nullptr )
						ppSubCell[i] = &(m_pCells[x+1][z]->SubCells[0][zz]);
					else
						ppSubCell[i] = nullptr;
					break;
				}

				if ( m_pCells[x][z] != nullptr )								
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx+1][zz]);
				else
					ppSubCell[i] = nullptr;					
				break;

			case 6:																		// x Áõ°¡. z °¨¼Ò.		
				if ( (x == (MAX_CELL_MAIN-1)) && (xx == (CELL_MAIN_DEVIDE-1)) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (z == 0) && (zz == 0) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (x != (MAX_CELL_MAIN-1)) && (xx == (CELL_MAIN_DEVIDE-1)) )			
				{
					if ( (z != 0) && (zz == 0) )
						if ( m_pCells[x+1][z-1] != nullptr )								
							ppSubCell[i] = &(m_pCells[x+1][z-1]->SubCells[0][CELL_MAIN_DEVIDE-1]);
						else
							ppSubCell[i] = nullptr;
					else
						if ( m_pCells[x+1][z] != nullptr )								
							ppSubCell[i] = &(m_pCells[x+1][z]->SubCells[0][zz-1]);
						else
							ppSubCell[i] = nullptr;
					break;
				}

				if ( (z != 0) && (zz == 0) )
				{
					if ( (x != (CELL_MAIN_SIZE-1)) && (xx == (CELL_MAIN_DEVIDE-1) ) )
						if ( m_pCells[x+1][z-1] != nullptr )								
							ppSubCell[i] = &(m_pCells[x+1][z-1]->SubCells[0][CELL_MAIN_DEVIDE-1]);
						else
							ppSubCell[i] = nullptr;
					else
						if ( m_pCells[x][z-1] != nullptr )								
							ppSubCell[i] = &(m_pCells[x][z-1]->SubCells[xx+1][CELL_MAIN_DEVIDE-1]);	
						else
							ppSubCell[i] = nullptr;
					break;
				}

				if ( m_pCells[x][z] != nullptr )								
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx+1][zz-1]);						
				else
					ppSubCell[i] = nullptr;					
				break;

			case 7:																		// z °¨¼Ò.
				if ( (z == 0) && (zz == 0) )	
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (z != 0) && (zz == 0) )
				{
					if ( m_pCells[x][z-1] != nullptr )								
						ppSubCell[i] = &(m_pCells[x][z-1]->SubCells[xx][CELL_MAIN_DEVIDE-1]);
					else
						ppSubCell[i] = nullptr;					
					break;
				}

				if ( m_pCells[x][z] != nullptr )								
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx][zz-1]);
				else
					ppSubCell[i] = nullptr;					
				break;

			case 8:																		// x °¨¼Ò, z °¨¼Ò.
				if ( (x == 0) && (xx == 0) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (z == 0) && (zz == 0) )
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ( (x != 0) && (xx == 0) )
				{
					if ( (z != 0) && (zz == 0) )
						if ( m_pCells[x-1][z-1] != nullptr )								
							ppSubCell[i] = &(m_pCells[x-1][z-1]->SubCells[CELL_MAIN_DEVIDE-1][CELL_MAIN_DEVIDE-1]);
						else
							ppSubCell[i] = nullptr;
					else
						if ( m_pCells[x-1][z] != nullptr )								
							ppSubCell[i] = &(m_pCells[x-1][z]->SubCells[CELL_MAIN_DEVIDE-1][zz-1]);
						else
							ppSubCell[i] = nullptr;
					break;
				}

				if ( (z != 0) && (zz == 0) )
				{
					if ( (x != 0) && (xx == 0) )
						if ( m_pCells[x-1][z-1] != nullptr )								
							ppSubCell[i] = &(m_pCells[x-1][z-1]->SubCells[CELL_MAIN_DEVIDE-1][CELL_MAIN_DEVIDE-1]);
						else
							ppSubCell[i] = nullptr;
					else
						if ( m_pCells[x][z-1] != nullptr )								
							ppSubCell[i] = &(m_pCells[x][z-1]->SubCells[xx-1][CELL_MAIN_DEVIDE-1]);	
						else
							ppSubCell[i] = nullptr;
					break;
				}
							
				if ( m_pCells[x][z] != nullptr )								
					ppSubCell[i] = &(m_pCells[x][z]->SubCells[xx-1][zz-1]);						
				else
					ppSubCell[i] = nullptr;
				break;
		}	// switch
	}	// for 
}

