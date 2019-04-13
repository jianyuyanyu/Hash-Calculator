#pragma once
#include "stdafx.h"

class Exception
{
public:
	LPCTSTR Message;

	Exception(LPCTSTR msg) :Message(msg) {}
};

class HashException :public Exception
{
public:
	HashException(LPCTSTR msg) :Exception(msg) {}
};

class URLException :public Exception
{
public:
	URLException(LPCTSTR msg) :Exception(msg) {}
};

class HttpException :public Exception
{
public:
	HttpException(LPCTSTR msg) :Exception(msg) {}
};
