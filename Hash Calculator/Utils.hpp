#pragma once
#pragma comment(lib,"Crypt32")

#include "stdafx.h"
#include <wincrypt.h>

namespace util
{
	// 0-15 转为字符 '0'-'9'，'A'-'F'
	inline TCHAR NumToChar(UCHAR num)
	{
		TCHAR ret = 0;
		if (num < 10)
		{
			ret = num + 48;
		}
		else
		{
			ret = num + 55;
		}
		return ret;
	}

	// '0'-'9'，'A'-'F' 转为数字 0-15，出错返回 255
	inline UCHAR CharToNum(TCHAR ch)
	{
		UCHAR num;
		if (ch >= 'a' && ch <= 'f')
		{
			num = ch - 87;
		}
		else if (ch >= 'A' && ch <= 'F')
		{
			num = ch - 55;
		}
		else if (ch >= '0' && ch <= '9')
		{
			num = ch - 48;
		}
		else
		{
			num = 255;
		}
		return num;
	}
}

inline CString ToString(LPCTSTR pFormat, ...)
{
	ASSERT(pFormat && pFormat != L"");
	va_list args;
	va_start(args, pFormat);
	CString str;
	str.FormatV(pFormat, args);
	va_end(args);
	return str;
}

inline void MsgBox(LPCTSTR pStr, LPCTSTR pCaption = NULL)
{
	MessageBox(NULL, pStr, pCaption, MB_OK);
}

inline CString BinToHex(const PUCHAR pBin, int len, BOOL bUpper = FALSE)
{
	static const TCHAR HexUpper[] = L"0123456789ABCDEF";
	static const TCHAR HexLower[] = L"0123456789abcdef";
	const TCHAR* hex = NULL;
	if (bUpper)
	{
		hex = HexUpper;
	}
	else
	{
		hex = HexLower;
	}
	CString ret;
	if (len <= 0) return ret;
	ret.Preallocate(len * 2);
	for (int i = 0; i < len; i++)
	{
		ret += hex[pBin[i] >> 4];
		ret += hex[pBin[i] & 0xF];
	}
	return ret;
}

inline int UcToMultiByteLen(UINT codepage, const CString& src, int len)
{
	return WideCharToMultiByte(codepage, 0, src, len, NULL, 0, NULL, NULL);
}

inline int UcToMultiByte(UINT codepage, const CString& src, int len, CByteArray* pByteArray)
{
	int needLen = UcToMultiByteLen(codepage, src, len);
	if (!needLen) return 0;
	pByteArray->SetSize(needLen);
	int retLen = WideCharToMultiByte(codepage, 0, src, len, (LPSTR)pByteArray->GetData(), needLen, NULL, NULL);
	pByteArray->SetSize(retLen);
	return retLen;
}

inline int UcToUTF8(const CString& src, int len, CByteArray* pByteArray)
{
	return UcToMultiByte(CP_UTF8, src, len, pByteArray);
}

inline int UcToANSI(const CString& src, int len, CByteArray* pByteArray)
{
	return UcToMultiByte(CP_ACP, src, len, pByteArray);
}

inline int UcFromMultiByteLen(UINT codepage, const PUCHAR pSrc, int len)
{
	return MultiByteToWideChar(codepage, 0, (LPCCH)pSrc, len, NULL, 0);
}

inline CString UcFromMultiByte(UINT codepage, const PUCHAR pSrc, int len)
{
	CString ret;
	int needLen = UcFromMultiByteLen(codepage, pSrc, len);
	if (!needLen) return ret;
	int retLen = MultiByteToWideChar(codepage, 0, (LPCCH)pSrc, len, ret.GetBuffer(needLen), needLen);
	ret.ReleaseBuffer(retLen);
	return ret;
}

inline CString UcFromUTF8(const PUCHAR pSrc, int len)
{
	return UcFromMultiByte(CP_UTF8, pSrc, len);
}

inline CString UcFromANSI(const PUCHAR pSrc, int len)
{
	return UcFromMultiByte(CP_ACP, pSrc, len);
}

inline CString UrlEncode(const CString& src)
{
	CString ret;
	CByteArray baUrl;
	int len = UcToUTF8(src, src.GetLength(), &baUrl);
	if (!len) return ret;
	UCHAR b = NULL;
	for (int i = 0; i < len; i++)
	{
		b = baUrl.GetAt(i);
		if (b >= 'a' && b <= 'z' || b >= 'A' && b <= 'Z' || b >= '0' && b <= '9')
		{
			ret += b;
			continue;
		}
		if (b == '.' || b == '*' || b == '_' || b == '-')
		{
			ret += b;
			continue;
		}
		if (b == ' ')
		{
			ret += '+';
			continue;
		}
		ret += '%';
		ret += util::NumToChar(b >> 4);
		ret += util::NumToChar(b & 15);
	}
	return ret;
}

inline CString UrlDecode(const CString& src)
{
	const int NEXT_NONE = 0;
	const int NEXT_HIGH = 1;
	const int NEXT_LOW = 2;
	CString ret;
	CByteArray baUrl;
	int len = UcToUTF8(src, src.GetLength(), &baUrl);
	if (!len) return ret;
	PUCHAR pChar = new UCHAR[len];
	int iSrc = 0;
	int iDest = 0;
	CHAR ch = 0;
	int flag = NEXT_NONE;
	UCHAR hChar = 0;
	UCHAR lChar = 0;
	while (iSrc < len)
	{
		ch = baUrl.GetAt(iSrc);
		iSrc++;
		if (flag == NEXT_NONE)
		{
			if (ch == '+')
			{
				pChar[iDest] = ' ';
				iDest++;
				continue;
			}
			if (ch == '%')
			{
				flag = NEXT_HIGH;
				continue;
			}
			pChar[iDest] = ch;
			iDest++;
			continue;
		}
		if (flag == NEXT_HIGH)
		{
			hChar = util::CharToNum(ch);
			if (hChar == 255) break;
			flag = NEXT_LOW;
			continue;
		}
		if (flag == NEXT_LOW)
		{
			lChar = util::CharToNum(ch);
			if (lChar == 255) break;
			pChar[iDest] = hChar << 4 | lChar;
			iDest++;
			flag = NEXT_NONE;
		}
	}
	ret = UcFromUTF8(pChar, iDest);
	delete[] pChar;
	return ret;
}

inline CString Base64Encode(const PUCHAR pSrc, int len)
{
	CString ret;
	if (len <= 0) return ret;
	DWORD needLen = len * 4 / 3 + 4;
	CryptBinaryToString(pSrc, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, ret.GetBuffer(needLen), &needLen);
	ret.ReleaseBuffer(needLen);
	return ret;
}

inline int Base64Decode(const CString& src, CByteArray* pByteArray)
{
	if (src.GetLength() <= 0) return 0;
	DWORD needLen = (src.GetLength() + 2) * 3 / 4;
	pByteArray->SetSize(needLen);
	CryptStringToBinary(src, src.GetLength(), CRYPT_STRING_BASE64, pByteArray->GetData(), &needLen, NULL, NULL);
	pByteArray->SetSize(needLen);
	return needLen;
}

inline LONGLONG GetCurrentTimeMillis()
{
	const LONGLONG base = 116444736000000000L;
	FILETIME fTime;
	GetSystemTimeAsFileTime(&fTime);
	LONGLONG timestamp = fTime.dwHighDateTime;
	timestamp <<= 32;
	timestamp |= fTime.dwLowDateTime;
	return (timestamp - base) / 10000;
}

inline DWORD GetCurrentTimeSeconds()
{
	return (DWORD)(GetCurrentTimeMillis() / 1000);
}

inline BOOL IsUserAdmin()
{
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);
	if (b)
	{
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
		{
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}
	return b;
}

inline VOID ChangeMessageFilterForDrop(HWND hwnd)
{
	ChangeWindowMessageFilterEx(hwnd, WM_DROPFILES, MSGFLT_ALLOW, NULL);
	ChangeWindowMessageFilterEx(hwnd, 0x0049, MSGFLT_ALLOW, NULL);	// WM_COPYGLOBALDATA
}

inline VOID RegisterDialogClass(HINSTANCE hInstance, LPCTSTR lpClassName)
{
	WNDCLASSEX ws{ sizeof(WNDCLASSEX) };
	GetClassInfoEx(hInstance, L"#32770", &ws);
	ws.lpszClassName = lpClassName;
	RegisterClassEx(&ws);
}
