#include "stdafx.h"
#include "ReferenceObject.h"

ReferenceObject::ReferenceObject() : m_refCount(0)
{
	IncRef();
}

void ReferenceObject::IncRef() 
{
	m_refCount.fetch_add(1, boost::memory_order_relaxed);
}

void ReferenceObject::DecRef()
{
	if (m_refCount.fetch_sub(1, boost::memory_order_release) == 1) 
	{
		boost::atomic_thread_fence(boost::memory_order_acquire);
		delete this;
	}
}