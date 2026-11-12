// Typed entity handle template that replaces the SDK EHANDLE class, moved from cbase.h
// Included from cbase.h in place of the SDK class

#pragma once

#include "extdll.h"

#include <functional>
#include <memory>
#include <unordered_set>

class CBaseEntity;

template <class ENTITY>
class EHandleT
{
	template <class ENTITY2>
	friend class EHandleT;

private:
	edict_t* m_pent{nullptr};
	int m_serialnumber{0};
	void* m_privateData{nullptr};

public:
	EHandleT() {}

	EHandleT(ENTITY* pEntity)
	{
		if (pEntity && pEntity->pev)
		{
			m_pent = ENT(pEntity->pev);
			if (m_pent)
			{
				m_serialnumber = m_pent->serialnumber;
				m_privateData = m_pent->pvPrivateData;
			}
		}
	}

	EHandleT(const EHandleT<ENTITY>& other)
	{
		m_pent = other.m_pent;
		m_serialnumber = other.m_serialnumber;
		m_privateData = other.m_privateData;
	}

	template <class ENTITY2>
	EHandleT(const EHandleT<ENTITY2>& other) : EHandleT{dynamic_cast<ENTITY*>(static_cast<CBaseEntity*>(const_cast<EHandleT<ENTITY2>&>(other).operator ENTITY2*()))}
	{
	}

	edict_t* Get() const
	{
		if (m_pent && m_pent->serialnumber == m_serialnumber && m_pent->pvPrivateData == m_privateData)
		{
			return m_pent;
		}
		return nullptr;
	}

	edict_t* Set(edict_t* pent)
	{
		m_pent = pent;
		if (pent)
		{
			m_serialnumber = m_pent->serialnumber;
			m_privateData = m_pent->pvPrivateData;
		}
		else
		{
			m_serialnumber = 0;
			m_privateData = nullptr;
		}
		return pent;
	}

	operator ENTITY*()
	{
		return static_cast<ENTITY*>(GET_PRIVATE(Get()));
	}

	operator const ENTITY*() const
	{
		return static_cast<const ENTITY*>(GET_PRIVATE(Get()));
	}

	operator int() const
	{
		return Get() != nullptr;
	}

	ENTITY* operator=(ENTITY* pEntity)
	{
		if (pEntity)
		{
			m_pent = ENT(pEntity->pev);
			if (m_pent)
			{
				m_serialnumber = m_pent->serialnumber;
				m_privateData = m_pent->pvPrivateData;
			}
		}
		else
		{
			m_pent = nullptr;
			m_serialnumber = 0;
			m_privateData = nullptr;
		}
		return pEntity;
	}

	ENTITY* operator->()
	{
		return static_cast<ENTITY*>(GET_PRIVATE(Get()));
	}

	const ENTITY* operator->() const
	{
		return static_cast<const ENTITY*>(GET_PRIVATE(Get()));
	}

	template <class ENTITY2>
	bool operator==(const EHandleT<ENTITY2>& other) const
	{
		return Get() == other.Get();
	}

	template <class ENTITY2>
	bool operator!=(const EHandleT<ENTITY2>& other) const
	{
		return !(operator==(other));
	}

	class Hash
	{
	public:
		std::size_t operator()(const EHandleT& e) const
		{
			std::hash<int> intHasher;
			std::hash<edict_t*> entHasher;
			std::hash<void*> ptrHasher;
			return intHasher(e.m_serialnumber) ^ entHasher(e.m_pent) ^ ptrHasher(e.m_privateData);
		}
	};

	class Equal
	{
	public:
		bool operator()(const EHandleT& e1, const EHandleT& e2) const
		{
			return e1.m_serialnumber == e2.m_serialnumber && e1.m_pent == e2.m_pent && e1.m_privateData == e2.m_privateData;
		}
	};
};

using EHANDLE = EHandleT<CBaseEntity>;
