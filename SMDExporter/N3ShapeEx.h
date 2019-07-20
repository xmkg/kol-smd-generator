#pragma once
#include <cwchar>
#include <string>
#include "My_3DStruct.h"
#include <vector>
#include <map>

typedef std::map<std::string, std::vector<std::string>> PartMap;
class CN3ShapeEx
{
public:
	CN3ShapeEx();
	~CN3ShapeEx();
	void Load(FILE * fp);
	PartMap & GetPartMap()  { return partMap; }

	__Vector3 & GetVPosition() { return m_vPos; }
	__Quaternion GetQRot() { return m_qRot; }
	__Vector3 & GetVScale() { return m_vScale; }

	std::string & GetName()  { return m_szName; }
	int GetBelongID() const { return m_iBelong; }
	int GetEventID() const { return m_iEventID; }
	int GetEventType() const { return m_iEventType; }
	int GetNpcID() const { return m_iNpcID; }
	int GetNpcStatus() const { return m_iNpcStatus; }

	unsigned __int64 GetOffset() const { return m_ulOffset; }

private:
	std::string m_szName;
	__Vector3 m_vPos;
	__Quaternion m_qRot;
	__Vector3 m_vScale;

	PartMap partMap;

	signed __int32 m_iBelong, m_iEventID, m_iEventType, m_iNpcID, m_iNpcStatus;
	unsigned __int64 m_ulOffset;

};


class CN3ShapePart
{
	// later
};

