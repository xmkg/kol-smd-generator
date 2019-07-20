#pragma once

#include "ReferenceObject.h"
#include "BCSTLMap.h"
#include "types.h"
#include <map>
#include <set>

#pragma pack(push, 1)
struct _OBJECT_EVENT
{
	// Indicates the object's belonging nation (0 = all, 1 = karus, 2 = elmo=
	int byNation;
	// The object's index.
	short sIndex;
	// Object type (gate,lever etc.)
	short sType;
	// The object's NPC ID
	short sControlNpcID;
	// Status flag for gate, lever like objects. (1 on, 0 off)
	short sStatus;
	float fPosX;
	float fPosY;
	float fPosZ;
	// Indicates if object is destroyed or not.
	uint8 byLife;
};

struct _REGENE_EVENT
{
	float fRegenePosX;
	float fRegenePosY;
	float fRegenePosZ;
	float fRegeneAreaZ;
	float fRegeneAreaX;
	int	  sRegenePoint;
};

struct _WARP_INFO
{
	short	sWarpID;
	char	strWarpName[32];
	char	strAnnounce[256];
	uint16	sUnk0; // padding?
	uint32	dwPay;
	short	sZone;
	uint16	sUnk1; // padding?
	float	fX;
	float	fY;
	float	fZ;
	float	fR;
	short	sNation;
	uint16	sUnk2; // padding?

	_WARP_INFO() { memset(this, 0, sizeof(_WARP_INFO)); };
};
#pragma pack(pop)

class CUser;

typedef BCSTLMap <_OBJECT_EVENT>		ObjectEventArray;
typedef BCSTLMap <_REGENE_EVENT>		ObjectRegeneArray;
typedef	BCSTLMap <_WARP_INFO>		    WarpArray;

class CN3ShapeMgr;
class SMDFile : public ReferenceObject
{
public:
	SMDFile();

	static SMDFile *Load(std::string mapName, bool bLoadWarpsAndRegeneEvents = false /* AI server doesn't need them */);

	void OnInvalidMap();
	bool LoadMap(FILE *fp, std::string & mapName, bool bLoadWarpsAndRegeneEvents /* AI server doesn't need them */);
	void LoadTerrain(FILE *fp);
	void LoadObjectEvent(FILE *fp);
	void LoadMapTile(FILE *fp);
	void LoadRegeneEvent(FILE *fp);
	void LoadWarpList(FILE *fp);

	bool IsValidPosition(float x, float z, float y);
	bool CheckEvent( float x, float z, CUser* pUser = nullptr );
	bool ObjectCollision(float x1, float z1, float y1, float x2, float z2, float y2);
	float GetHeight( float x, float y, float z );

	int GetEventID(int x, int z);

	CN3ShapeMgr * GetShapeMgr() const { return m_N3ShapeMgr; }

	INLINE int GetMapSize() { return m_nMapSize - 1; }
	INLINE float GetUnitDistance() { return m_fUnitDist; }
	INLINE int GetXRegionMax() { return m_nXRegion - 1; }
	INLINE int GetZRegionMax() { return m_nZRegion - 1; }

	INLINE short * GetEventIDs() { return m_ppnEvent; }

	INLINE ObjectEventArray	* GetObjectEventArray() { return &m_ObjectEventArray; }
	INLINE _OBJECT_EVENT * GetObjectEvent(int objectindex) { return m_ObjectEventArray.GetData(objectindex); }
	INLINE _REGENE_EVENT * GetRegeneEvent(int objectindex) { return m_ObjectRegeneArray.GetData(objectindex); }
	INLINE _WARP_INFO * GetWarp(int warpID) { return m_WarpArray.GetData(warpID); }

	void GetWarpList(int warpGroup, std::set<_WARP_INFO *> & warpEntries);

	short GetMapTile(int x, int z){ return m_ppnEvent[x*z]; }

	virtual ~SMDFile();

private:
	std::string m_MapName;

	short*		m_ppnEvent;
	WarpArray	m_WarpArray;

	ObjectEventArray	m_ObjectEventArray;
	ObjectRegeneArray	m_ObjectRegeneArray;

	CN3ShapeMgr *m_N3ShapeMgr;

	float*		m_fHeight;

	int			m_nXRegion, m_nZRegion;

	int			m_nMapSize;		// Grid Unit ex) 4m
	float		m_fUnitDist;	// i Grid Distance

	typedef std::map<std::string, void *> SMDMap;
	static SMDMap  s_loadedMaps;


	friend class C3DMap;

};
