#pragma once
#include "stdafx.h"
#include "File.hpp"

class HashListener
{
public:
	virtual void OnBegin(LPCTSTR path, File& f) = 0;
	virtual void OnProgress(int rating) = 0;
	virtual void OnCompleted(PBYTE pResult, DWORD len, DWORD time) = 0;
	virtual void OnError(LPCTSTR path, LPCTSTR error) = 0;
};
