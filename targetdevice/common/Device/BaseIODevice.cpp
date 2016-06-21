#include "StdAfx.h"
#include "BaseIODevice.h"


CBaseIODevice::CBaseIODevice(void)
{
	m_pLogger = NULL;
}


CBaseIODevice::~CBaseIODevice(void)
{
}

void CBaseIODevice::Log(LPCTSTR s)
{
	if (m_pLogger != NULL)
	{
		try
		{
			m_pLogger->Log(s);
		}
		catch (...)
		{

		}
	}
}