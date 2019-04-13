#pragma once
#include "stdafx.h"
#include "Exception.hpp"

class IOCP
{
private:
	HANDLE m_handle;

public:
	IOCP() :m_handle(NULL) {}
	IOCP(const IOCP&) = delete;
	IOCP& operator = (const IOCP&) = delete;
	IOCP(IOCP&& iocp)
	{
		iocp.m_handle = m_handle;
		m_handle = NULL;
	}
	~IOCP()
	{
		Close();
	}
	BOOL Create(DWORD NumberOfConcurrentThreads = 0)
	{
		ASSERT(!m_handle);
		m_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, NumberOfConcurrentThreads);
		return m_handle != NULL;
	}
	void Associate(HANDLE FileHandle, ULONG_PTR CompletionKey)
	{
		HANDLE handle = CreateIoCompletionPort(FileHandle, m_handle, CompletionKey, 0);
		ASSERT(handle == m_handle);
	}
	HANDLE GetHandle()
	{
		return m_handle;
	}
	// 所有参数都不可为 NULL
	BOOL Get(LPDWORD lpNumberOfBytes, PULONG_PTR lpCompletionKey, LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds = INFINITE)
	{
		return GetQueuedCompletionStatus(m_handle, lpNumberOfBytes, lpCompletionKey, lpOverlapped, dwMilliseconds);
	}
	BOOL Post(DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped)
	{
		return PostQueuedCompletionStatus(m_handle, dwNumberOfBytesTransferred, dwCompletionKey, lpOverlapped);
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
