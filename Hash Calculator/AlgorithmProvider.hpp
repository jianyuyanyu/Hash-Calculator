#pragma once
#pragma comment(lib,"Bcrypt")

#include "stdafx.h"
#include <bcrypt.h>

class AlgorithmProvider
{
private:
	BCRYPT_ALG_HANDLE m_handle;

public:
	AlgorithmProvider() :m_handle(NULL) {}
	AlgorithmProvider(LPCWSTR pszAlgId, ULONG dwFlags = NULL) :m_handle(NULL)
	{
		Open(pszAlgId, dwFlags);
	}
	AlgorithmProvider(const AlgorithmProvider&) = delete;
	AlgorithmProvider& operator = (const AlgorithmProvider&) = delete;
	AlgorithmProvider(AlgorithmProvider&& ap) :m_handle(ap.m_handle)
	{
		ap.m_handle = NULL;
	}
	~AlgorithmProvider()
	{
		Close();
	}
	void Close()
	{
		if (m_handle)
		{
			BCryptCloseAlgorithmProvider(m_handle, 0);
			m_handle = NULL;
		}
	}
	BCRYPT_HASH_HANDLE CreateHash()
	{
		BCRYPT_HASH_HANDLE pHash = NULL;
		NTSTATUS status = BCryptCreateHash(m_handle, &pHash, NULL, 0, NULL, 0, 0);
		ASSERT(status >= 0);
		return pHash;
	}
	BCRYPT_HASH_HANDLE CreateHash(LPCVOID pbSecret, ULONG cbSecret)
	{
		ASSERT(pbSecret);
		ASSERT(cbSecret > 0);
		BCRYPT_HASH_HANDLE pHash = NULL;
		NTSTATUS status = BCryptCreateHash(m_handle, &pHash, NULL, 0, (PUCHAR)pbSecret, cbSecret, 0);
		ASSERT(status >= 0);
		return pHash;
	}
	BCRYPT_KEY_HANDLE GenerateSymmetricKey(LPCVOID pbSecret, ULONG cbSecret)
	{
		ASSERT(pbSecret);
		ASSERT(cbSecret > 0);
		BCRYPT_KEY_HANDLE hKey = NULL;
		NTSTATUS status = BCryptGenerateSymmetricKey(m_handle, &hKey, NULL, 0, (PUCHAR)pbSecret, cbSecret, 0);
		ASSERT(status >= 0);
		return hKey;
	}
	BCRYPT_ALG_HANDLE GetHandle()
	{
		return m_handle;
	}
	DWORD GetProperty(LPCWSTR pszProperty, LPVOID pbOutput, ULONG cbOutput)
	{
		ASSERT(pbOutput);
		ASSERT(cbOutput > 0);
		DWORD cbData = 0;
		NTSTATUS status = BCryptGetProperty(m_handle, pszProperty, (PUCHAR)pbOutput, cbOutput, &cbData, 0);
		ASSERT(status >= 0);
		return cbData;
	}
	BOOL Open(LPCWSTR pszAlgId, ULONG dwFlags = NULL)
	{
		ASSERT(!m_handle);
		NTSTATUS status = BCryptOpenAlgorithmProvider(&m_handle, pszAlgId, NULL, dwFlags);
		return status >= 0;
	}
	BOOL SetProperty(LPCWSTR pszProperty, PUCHAR pbInput, ULONG cbInput)
	{
		ASSERT(pbInput);
		ASSERT(cbInput > 0);
		NTSTATUS status = BCryptSetProperty(m_handle, pszProperty, pbInput, cbInput, NULL);
		return status >= 0;
	}

};
