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
#include <boost/atomic.hpp>
#include "types.h"


class ReferenceObject
{
public:
	ReferenceObject();

	// Increment the reference count
	void IncRef();

	// Decrease the reference count and delete the object if it hits 0 (i.e. no more references).
	void DecRef();

	virtual ~ReferenceObject() {}
   
private:
	mutable boost::atomic<uint32> m_refCount;
};