#pragma once
#include <string>
#include "N3ShapeMgr.h"

class CObjectPostData
{
public:
	explicit CObjectPostData();
	~CObjectPostData();

	bool Load(const std::string & szPath);
	bool Save() = delete;
	void DoCleanup();
	CN3ShapeMgr * GetShapeMgr() { return &m_ShapeMgr; }
	bool m_bOldFileMode;
private:
	CN3ShapeMgr m_ShapeMgr;
	std::string ReadDecryptString(FILE * fp);
};

