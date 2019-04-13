#pragma once

#include "stdafx.h"
#include "AlgorithmProvider.hpp"
#include "Exception.hpp"
#include <memory>

class Hash
{
private:
	AlgorithmProvider m_alg;
	BCRYPT_HASH_HANDLE m_handle;
	DWORD m_len;

public:
	// 出错会抛出 HashException 异常
	Hash(LPCWSTR pszAlgId) :m_alg(pszAlgId), m_handle(NULL), m_len(NULL)
	{
		m_alg.GetProperty(BCRYPT_HASH_LENGTH, &m_len, sizeof(DWORD));
		Reset();
	}
	// 出错会抛出 HashException 异常
	Hash(LPCWSTR pszAlgId, LPCVOID pbSecret, ULONG cbSecret) :m_alg(pszAlgId, BCRYPT_ALG_HANDLE_HMAC_FLAG),
		m_handle(NULL), m_len(NULL)
	{
		m_alg.GetProperty(BCRYPT_HASH_LENGTH, &m_len, sizeof(DWORD));
		Reset(pbSecret, cbSecret);
	}
	Hash(const Hash&) = delete;
	Hash& operator = (const Hash&) = delete;
	Hash(Hash&& hash) :m_alg(std::move(hash.m_alg))
	{
		m_handle = hash.m_handle;
		hash.m_handle = NULL;
		m_len = hash.m_len;
		hash.m_len = 0;
	}
	~Hash()
	{
		if (m_handle)
		{
			BCryptDestroyHash(m_handle);
			m_handle = NULL;
		}
		m_len = 0;
	}
	BCRYPT_HASH_HANDLE GetHandle()
	{
		return m_handle;
	}
	// 出错会抛出 HashException 异常
	Hash& Reset()
	{
		if (m_handle) BCryptDestroyHash(m_handle);
		m_handle = m_alg.CreateHash();
		if (!m_handle) throw HashException(L"Hash.Reset: CreateHash Failed");

		return *this;
	}
	// 出错会抛出 HashException 异常
	Hash& Reset(LPCVOID pbSecret, ULONG cbSecret)
	{
		if (m_handle) BCryptDestroyHash(m_handle);
		m_handle = m_alg.CreateHash(pbSecret, cbSecret);
		if (!m_handle) throw HashException(L"Hash.Reset: CreateHash Failed");

		return *this;
	}
	// 出错会抛出 HashException 异常
	Hash& Update(LPCVOID pbInput, DWORD cbInput)
	{
		ASSERT(pbInput);
		NTSTATUS status = BCryptHashData(m_handle, (PUCHAR)pbInput, cbInput, 0);
		if (status < 0) throw HashException(L"Hash.Update: HashData Failed");

		return *this;
	}
	// 出错会抛出 HashException 异常
	std::unique_ptr<UCHAR> GetValue()
	{
		std::unique_ptr<UCHAR> buffer(new UCHAR[m_len]);
		NTSTATUS status = BCryptFinishHash(m_handle, buffer.get(), m_len, 0);
		if (status < 0) throw HashException(L"Hash.GetValue: FinishHash Failed");
		return buffer;
	}
	DWORD GetLength()
	{
		return m_len;
	}
};
