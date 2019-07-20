#pragma once

#	include <cassert>

#	define ASSERT assert
#	define TRACE FormattedDebugString

void FormattedDebugString(const char * fmt, ...);