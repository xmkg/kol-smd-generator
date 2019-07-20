#pragma once
#include <string>

class CGameTile
{
public:
	CGameTile();
	~CGameTile();
	bool Load(const std::string & szFileName);
};

