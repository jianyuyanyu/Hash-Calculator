#pragma once
#include "stdafx.h"

class File
{
private:
	HANDLE m_handle;

public:
	File() :m_handle(NULL) {}
	File(LPCTSTR lpFileName, DWORD dwDesiredAccess, BOOL bAutoCreate = FALSE,
		DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL) :m_handle(NULL)
	{
		Open(lpFileName, dwDesiredAccess, bAutoCreate, dwFlagsAndAttributes);
	}
	File(const File&) = delete;
	File& operator = (const File&) = delete;
	File(File&& file) :m_handle(file.m_handle)
	{
		file.m_handle = NULL;
	}
	~File()
	{
		Close();
	}
	static DWORD ReadAll(LPCTSTR lpFileName, CByteArray* pByteArray)
	{
		ASSERT(pByteArray);
		File file(lpFileName, GENERIC_READ);
		auto len = (DWORD)file.Length();
		if (!len) return 0;
		pByteArray->SetSize(len);
		auto readLen = file.Read(pByteArray->GetData(), len);
		pByteArray->SetSize(readLen);
		return readLen;
	}
	static DWORD WriteAll(LPCTSTR lpFileName, LPVOID lpBuffer, DWORD len)
	{
		File file(lpFileName, GENERIC_WRITE);
		file.SetEnd();
		return file.Write(lpBuffer, len);
	}
	HANDLE GetHandle()
	{
		return m_handle;
	}
	// 不需要获取的参数可以设置为 NULL
	BOOL GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime)
	{
		return ::GetFileTime(m_handle, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
	}
	// 不需要设置的参数可以设置为 NULL
	BOOL SetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime)
	{
		return ::SetFileTime(m_handle, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
	}
	BOOL Open(LPCTSTR lpFileName, DWORD dwDesiredAccess, BOOL bAutoCreate = FALSE, DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL)
	{
		ASSERT(!m_handle);
		ASSERT(lpFileName);
		HANDLE handle = CreateFile(lpFileName, dwDesiredAccess, FILE_SHARE_READ, NULL,
			bAutoCreate ? OPEN_ALWAYS : OPEN_EXISTING, dwFlagsAndAttributes, NULL);
		if (handle == INVALID_HANDLE_VALUE) return false;
		m_handle = handle;
		return true;
	}
	BOOL FlushBuffer()
	{
		return FlushFileBuffers(m_handle);
	}
	LONGLONG Length()
	{
		LARGE_INTEGER li{};
		GetFileSizeEx(m_handle, &li);
		return li.QuadPart;
	}
	DWORD Read(LPVOID lpBuffer, DWORD len)
	{
		ASSERT(lpBuffer);
		ASSERT(len);
		DWORD readLen = 0;
		ReadFile(m_handle, lpBuffer, len, &readLen, NULL);
		return readLen;
	}
	// 如果该操作同步完成则返回 true
	BOOL ReadAsync(LPVOID lpBuffer, DWORD len, LPOVERLAPPED lpOverlapped)
	{
		ASSERT(lpBuffer);
		ASSERT(len);
		ASSERT(lpOverlapped);
		return ReadFile(m_handle, lpBuffer, len, NULL, lpOverlapped);
	}
	// 从开始位置移动到 position 处，返回新位置
	LONGLONG Seek(LONGLONG position)
	{
		LARGE_INTEGER li{};
		li.QuadPart = position;
		LARGE_INTEGER newPos{};
		SetFilePointerEx(m_handle, li, &newPos, FILE_BEGIN);
		return newPos.QuadPart;
	}
	BOOL SetEnd() { return SetEndOfFile(m_handle); }
	// 从当前位置跳过 n 个字节，返回新位置
	LONGLONG Skip(LONGLONG n)
	{
		LARGE_INTEGER li{};
		li.QuadPart = n;
		LARGE_INTEGER newPos{};
		SetFilePointerEx(m_handle, li, &newPos, FILE_CURRENT);
		return newPos.QuadPart;
	}
	BOOL WaitFor(DWORD dwMilliseconds = INFINITE)
	{
		return WaitForSingleObject(m_handle, dwMilliseconds) == WAIT_OBJECT_0;
	}
	DWORD Write(LPVOID lpBuffer, DWORD len)
	{
		ASSERT(lpBuffer);
		ASSERT(len);
		DWORD writeLen = 0;
		WriteFile(m_handle, lpBuffer, len, &writeLen, NULL);
		return writeLen;
	}
	// 如果该操作同步完成则返回 true
	BOOL WriteAsync(LPVOID lpBuffer, DWORD len, LPOVERLAPPED lpOverlapped)
	{
		ASSERT(lpBuffer);
		ASSERT(len);
		ASSERT(lpOverlapped);
		return WriteFile(m_handle, lpBuffer, len, NULL, lpOverlapped);
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
