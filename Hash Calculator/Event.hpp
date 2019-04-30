#pragma once
#include "stdafx.h"

class Event
{
private:
	HANDLE m_handle;

public:
	Event() :m_handle(NULL) {}
	Event(BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName = NULL, BOOL* pbAlreadyExist = NULL) :m_handle(NULL)
	{
		Create(bManualReset, bInitialState, lpName, pbAlreadyExist);
	}
	Event(const Event&) = delete;
	Event& operator = (const Event&) = delete;
	Event(Event&& e) :m_handle(e.m_handle)
	{
		e.m_handle = NULL;
	}
	~Event()
	{
		Close();
	}
	BOOL Create(BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName = NULL, BOOL* pbAlreadyExist = NULL)
	{
		ASSERT(!m_handle);
		m_handle = CreateEvent(NULL, bManualReset, bInitialState, lpName);
		if (pbAlreadyExist) *pbAlreadyExist = GetLastError() == ERROR_ALREADY_EXISTS;
		return m_handle != NULL;
	}
	HANDLE GetHandle()
	{
		return m_handle;
	}
	BOOL Set()
	{
		return SetEvent(m_handle);
	}
	BOOL Reset()
	{
		return ResetEvent(m_handle);
	}
	BOOL Pulse()
	{
		return PulseEvent(m_handle);
	}
	BOOL WaitFor(DWORD dwMilliseconds = INFINITE)
	{
		return WaitForSingleObject(m_handle, dwMilliseconds) == WAIT_OBJECT_0;
	}
	void Close()
	{
		if (m_handle)
		{
			CloseHandle(m_handle);
			m_handle = NULL;
		}
	}

};
