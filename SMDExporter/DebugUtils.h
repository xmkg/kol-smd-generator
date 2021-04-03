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

#	include <cassert>

#	define ASSERT assert
#	define TRACE FormattedDebugString

void FormattedDebugString(const char * fmt, ...);