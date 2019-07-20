// SMDExporter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <filesystem>
#include <boost/algorithm/string.hpp>
#include "tstring.h"
#include "GameserverMap.h"

/*
	Server map data generator
	copyright (c) 2015 PENTAGRAM.
	All rights reserved.
*/

/*
	TODO : Append original file SHA1 information into the GSMD header.
*/

static std::vector<std::string> mapNames;

static void init_mapnames()
{
	/**/

	std::string path = "opd";
	try {
		for (auto & p : std::filesystem::directory_iterator(path)) {
			if (p.is_regular_file()) {
				if (p.path().has_filename() && p.path().has_extension() && boost::iequals(p.path().extension().string(), ".opd")) {
					
					std::string fname = p.path().filename().string();
					boost::replace_all(fname, ".opd", "");
					mapNames.push_back(fname);
				}

			}

		}
	}
	catch (std::exception & ex) {
		printf("map name obtain attempt failed.");
		mapNames.push_back("arena");
		mapNames.push_back("battlezone");
		mapNames.push_back("battlezone_b");
		mapNames.push_back("battlezone_c");
		mapNames.push_back("battlezone_d");
		mapNames.push_back("battlezone_e");
		mapNames.push_back("battlezone_f");
		mapNames.push_back("clanfight");
		mapNames.push_back("clanfight_b");
		mapNames.push_back("clanfight_c");
		mapNames.push_back("dragon_a");
		mapNames.push_back("dungeon_a");
		mapNames.push_back("dungeon_b1th");
		mapNames.push_back("dungeon_b2th");
		mapNames.push_back("dungeond");
		mapNames.push_back("elmo2004");
		mapNames.push_back("elmorad_intro");
		mapNames.push_back("elmorad_start");
		mapNames.push_back("eslantzone");
		mapNames.push_back("freezone");
		mapNames.push_back("freezone_a");
		mapNames.push_back("freezone_b");
		mapNames.push_back("freezone_bb");
		mapNames.push_back("freezone_new");
		mapNames.push_back("freezone_c");
		mapNames.push_back("in_dungeon01");
		mapNames.push_back("in_dungeon02");
		mapNames.push_back("in_dungeon03");
		mapNames.push_back("in_dungeon04");
		mapNames.push_back("in_dungeon05");
		mapNames.push_back("karus_intro");
		mapNames.push_back("karus_start");
		mapNames.push_back("karus2004");
		mapNames.push_back("moradon");
		mapNames.push_back("moradon_xmas");
		mapNames.push_back("war_a");
		mapNames.push_back("ch_qgs");
		mapNames.push_back("bossmode");
		mapNames.push_back("new_runawar");
		mapNames.push_back("sky_war_2009");
		mapNames.push_back("old_karus");
		mapNames.push_back("old_elmo");

		mapNames.push_back("freezone");
		mapNames.push_back("itemzone_a");
		mapNames.push_back("dungeon_defense");
		mapNames.push_back("dungeon_b3th2015");
	}

}

void make_gsmdfiles()
{
	printf("Generating GSMD files..\n");

	for (const auto & val : mapNames)
	{
		printf("** Making %s.gsmd..**\n", val.c_str());
		std::string tileFileName = string_format(".\\tile\\%s%s", val.c_str(), ".tile");
		std::string gsmapFileName = string_format(".\\gsmd\\%s%s", val.c_str(), ".gsmd");
		CGameserverMap * gsMap;

	

		gsMap = CGameserverMap::Load(string_format("%s.opd", val.c_str()), string_format("%s.gtd", val.c_str()));
		if(gsMap)
		{
			gsMap->LoadAndWriteExternalTile(tileFileName);
			gsMap->SaveGameserverMapData(gsmapFileName);
			printf("** Done making %s.gsmd **\n", val.c_str());
		/*	if (val == "moradon")
				__debugbreak();*/
		}
		else
		{
			printf("!! GSMD generation failed for %s !!\n", val.c_str());
		}
		delete gsMap;
	}

	printf("GSMD generation finished.\n");

}

void check_allmap()
{
	int successCount = 0;
	std::vector<std::string> invalidMapFiles;
	for (const auto & val : mapNames)
	{
		printf("** Validating %s.gsmd..**\n", val.c_str());
		std::string fname = string_format(".\\gsmd\\%s.gsmd", val.c_str());
		CGameserverMap cgsm;
		if (cgsm.LoadGameserverMapData(fname))
		{
			printf(" --- file load succeeded \n");
			printf(" --- real name %s, author %s, tool ver. %d.%d, map size %d\n", cgsm.GetGSMDHeader().szMapName.c_str(), cgsm.GetGSMDHeader().szAuthor.c_str(), cgsm.GetGSMDHeader().VERSION_HI, cgsm.GetGSMDHeader().VERSION_LO, cgsm.GetMapSize());
			printf(" --- object event count : %d\n", cgsm.GetObjectEvents().GetSize());
			printf(" --- creation date %s", asctime(localtime(reinterpret_cast<const long long*>(&cgsm.GetGSMDHeader().ulCreationDate))));
			++successCount;
		}
		else {
			printf(" ---  file load failed.\n");
			invalidMapFiles.push_back(val);
		}
	}

	printf("%d out of %llu gameserver map data files are valid.\n", successCount, mapNames.size());
	for (const auto & val : invalidMapFiles) {
		printf(" --- %s is invalid.\n", val.c_str());
	}

}

int main()
{
	init_mapnames();
	make_gsmdfiles();
	check_allmap();


	/**/

	system("pause");
    return 0;
}

