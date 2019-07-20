// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include "DebugUtils.h"
#include "types.h"

#define GSMD_TOOL

#define foreach_auto(arr)\
	for(const auto& elem: arr)

// Iterates over each element in given STLMap and locks the map mutex.
#define foreach_stlmap(itr, arr) \
	BCSTLMap<void>::Guard lock(arr._access); \
	foreach_stlmap_nolock(itr, arr)

// Iterates over each element in given STLMap.
#define foreach_stlmap_nolock(itr, arr) \
	for (auto itr = arr.m_UserTypeMap.begin(); itr != arr.m_UserTypeMap.end(); itr++)


#define OPD_PATH "./opd/"
#define GTD_PATH "./gtd/"
#define TILE_PATH "./tile/"


// TODO: reference additional headers your program requires here
