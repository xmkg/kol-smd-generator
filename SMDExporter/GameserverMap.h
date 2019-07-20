#pragma once
#include <string>

#ifdef GSMD_TOOL
	#include "ObjectPostData.h"
	#include "GameTerrain.h"
#else
	#include "../N3BASE/ObjectPostData.h"
	#include "../N3BASE/GameTerrain.h"
#endif

#include "BCSTLMap.h"

#ifndef GSMD_TOOL
	#include "database/structs.h"
#endif


/* Version 1.1 */
const static uint8 GAMESERVER_MAP_VERSION_HI = 1;
const static uint8 GAMESERVER_MAP_VERSION_LO = 2;

#ifdef GSMD_TOOL
struct _OBJECT_EVENT
{
	// The object's index.
	short sObjectID;
	// The object's NPC ID
	short sAssociatedNpcID;
	// Object type (gate,lever etc.)
	uint8 byType;
	// Indicates the object's belonging nation (0 = all, 1 = karus, 2 = elmo=
	uint8 byNation;
	// Status flag for gate, lever like objects. (1 on, 0 off)
	uint8 byStatus;
	float fPosX;
	float fPosY;
	float fPosZ;
	// Indicates if object is destroyed or not.
	uint8 isAlive;
};
#endif
/*
	Definition of the gameserver map file.
	This file consists all object & terrain related operations and controls.
*/
typedef BCSTLMap<_OBJECT_EVENT>		ObjectEventArray;

const static float fUnitDistance = 4.0f;
#define VIEW_DISTANCE 48

struct _GSMD_HEADER
{
	std::string szMapName;
	std::string szAuthor;
	uint64      ulCreationDate;
	uint8		VERSION_HI;
	uint8		VERSION_LO;

	void WriteHeader(FILE * fp)
	{
		int szML = strlen(szMapName.c_str());
		int szAL = strlen(szAuthor.c_str());
		fwrite(&szML, 4, 1, fp);
		for (const auto & c : szMapName)
			fwrite(&c, 1, 1, fp);
		fwrite(&szAL, 4, 1, fp);
		for (const auto & c : szAuthor)
			fwrite(&c, 1, 1, fp);

		fwrite(&VERSION_HI, 1, 1, fp);
		fwrite(&VERSION_LO, 1, 1, fp);
		fwrite(&ulCreationDate, 8, 1, fp);
	}

	void ReadHeader(FILE * fp)
	{
		int szML, szAL;
		fread_s(&szML, 4, 4, 1, fp);
		std::vector<char> buffer_1(szML + 1, NULL);
		fread_s(&(buffer_1[0]), szML, 1, szML, fp);
		std::string mapName(buffer_1.begin(), buffer_1.end());
		szMapName = mapName;

		fread_s(&szAL, 4, 4, 1, fp);
		std::vector<char> buffer_2(szAL + 1, NULL);
		fread_s(&(buffer_2[0]), szAL, 1, szAL, fp);
		std::string authorName(buffer_2.begin(), buffer_2.end());
		szAuthor = authorName;

		fread_s(&VERSION_HI, 1, 1, 1, fp);
		fread_s(&VERSION_LO, 1, 1, 1, fp);
		fread_s(&ulCreationDate, 8, 8, 1, fp);
	}
};
/*
Definition of the gameserver map file.
This file consists all object & terrain related operations and controls.
*/
class CGameserverMap
{
public:
	static CGameserverMap * Load(const std::string & szOpdFile, const std::string&  szGtdFile);
	static void OnInvalidMap();
	CGameserverMap();
	~CGameserverMap();

	bool LoadObjectPostData(const std::string & szOpdFile);
	bool LoadTerrainData(const std::string & szGtdFile);


	INLINE int GetMapSize() const { return m_TerrainData.GetMapSize() - 1; }
	INLINE float GetUnitDistance() { return fUnitDistance; }
	INLINE int GetXRegionMax() { return m_nXRegion - 1; }
	INLINE int GetZRegionMax() { return m_nZRegion - 1; }

	INLINE int Coordinate2Tile(const float fV) const { return static_cast<int>(fV / 4); }
	//INLINE int GetTileZByCoord(const float fZ) { return static_cast<int>(fX / 4); }

	int GetEvent(const float fPosX, const float fPosZ)const;
	int GetEvent(__Vector3 vPos)const;

	/// to be implemented later
	bool CheckCollision(const float fPosX, const float fPosY, const float fPosZ) = delete;
	bool CheckCollision(__Vector3 & vPosOld, __Vector3 & vPosNew);
	bool ValidatePosition(const float fX, const float fY, const float fZ);
	bool ValidatePosition(__Vector3 & vPos);
	float GetHeightBy2DPos(const float fX, const float fZ);
	int GetEventByPos(__Vector3 & vPos) = delete;
	bool GetEventByPos(const float fPosX, const float fPosY, const float fPosZ) = delete;
	int m_nXRegion, m_nZRegion;

	_OBJECT_EVENT * GetObjectEvent(int iObjectID);

	ObjectEventArray & GetObjectEvents() { return m_ObjectEvents; }

	CN3ShapeMgr * GetShapeMgr() { return m_ObjectPostData.GetShapeMgr(); }

	/*
	Exports move table information to a file.
	*/
	bool SaveMoveTable(const std::string & szFileName);
	bool SaveMoveTableToStream(FILE *fp);

	/*
		Save combined mapdata to file
	*/
	bool SaveGameserverMapData(const std::string & szFileName);

	void LoadAndWriteExternalTile(const std::string & szFileName);

	/*
		Load from previously saved *.GSMD file
	*/
	bool LoadGameserverMapData(const std::string & szFileName);
	_GSMD_HEADER & GetGSMDHeader() { return m_GSMDHeader; }
private:
	CObjectPostData m_ObjectPostData;
	CGameTerrain    m_TerrainData;
	ObjectEventArray m_ObjectEvents;
	_GSMD_HEADER     m_GSMDHeader;
	short ** m_ppEvents;
	
	/*
		Checks if the map is valid or not.
	*/
	bool Valid();
	/*
		Initializes the variables, required fields etc.
	*/
	void Initialize(bool isGSMD = false);
	/*
		Generates the object event array required for the zone object operation.
	*/
	void GenerateObjectEvents();
	/*
		Generates move tile table according to loaded OPD and GTD files.
	*/
	void GenerateMoveTable();
	/*
		Cleans up unrequired resources.
	*/

	void Cleanup();

	int m_iMapSize;



	//unsigned __int8 VERSION_HI, VERSION_LO;

};
