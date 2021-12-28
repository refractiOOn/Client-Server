#pragma once
#include "windows.h"

class HandleWrapper
{
public:
	HandleWrapper(HANDLE handle);
	~HandleWrapper();
	HANDLE Get() const;
private:
	HANDLE m_handle = INVALID_HANDLE_VALUE;
};