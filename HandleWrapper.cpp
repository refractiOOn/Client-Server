#include "HandleWrapper.h"

HandleWrapper::HandleWrapper(HANDLE handle) :
	m_handle(handle)
{

}

HandleWrapper::~HandleWrapper()
{
	if (m_handle && m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
	}
}

HANDLE HandleWrapper::Get() const
{
	return m_handle;
}
