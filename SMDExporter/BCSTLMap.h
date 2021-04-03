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

#include <boost/unordered_map.hpp>
#include <boost/thread/locks.hpp>
#include <boost/foreach.hpp>
#include <boost/iterator.hpp>
#include <boost/thread/recursive_mutex.hpp>

using uint32 = unsigned __int32;
//using Mutex = boost::recursive_mutex;
#define VCALL __fastcall


template <class T>
class BCSTLMap
{
public:
	typedef boost::unordered_map<unsigned __int32, T*> Map;
	typedef typename boost::unordered_map<unsigned __int32, T*>::iterator Iterator;

	typedef typename boost::recursive_mutex Mutex;
	typedef  boost::recursive_mutex::scoped_lock Guard;
	Mutex _access;
	Map m_UserTypeMap;

	int32_t VCALL GetSize()
	{
		Guard lock(_access);
		return m_UserTypeMap.size();
	}

	bool VCALL IsExist(uint32 key)
	{
		Guard lock(_access);
		return (m_UserTypeMap.find(key) != m_UserTypeMap.end());
	}

	bool VCALL IsEmpty()
	{
		Guard lock(_access);
		return m_UserTypeMap.empty();
	}

	bool VCALL PutData(uint32 key_value, T* pData)
	{
		Guard lock(_access);
		//	printf("inserted key %d\n", key_value);
		return m_UserTypeMap.insert(std::make_pair(key_value, pData)).second;
	}



	T* VCALL GetData(uint32 key_value)
	{
		Guard lock(_access);
		auto itr = m_UserTypeMap.find(key_value);
		return (itr != m_UserTypeMap.end() ? itr->second : nullptr);
	}

	void VCALL DeleteData(uint32 key_value)
	{
		Guard lock(_access);
		auto itr = m_UserTypeMap.find(key_value);
		if (itr != m_UserTypeMap.end())
		{
			delete itr->second;
			m_UserTypeMap.erase(itr);
		}
	}

	void VCALL DeleteAllData()
	{
		Guard lock(_access);
		if (m_UserTypeMap.empty())
			return;
		BOOST_FOREACH(Map::value_type pair, m_UserTypeMap)
		{
			delete pair.second;
		}


		m_UserTypeMap.clear();
	}

	~BCSTLMap() { DeleteAllData(); }

	

};