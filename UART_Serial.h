#pragma once
#include "Dark_Transport.h"

class UART_Serial : public Dark_Transport
{
public:
	UART_Serial();
	virtual ~UART_Serial();

	virtual int32_t dark_read(uint8_t* pBuf, int32_t size);
	virtual int32_t dark_write(uint8_t* pBuf, int32_t size);

	int32_t Open(void* pPort, int32_t iBaudRate, char cParity, int32_t iDataBit, int32_t iStopBit);
	int32_t Close();

protected:
#ifdef _MSC_VER
	HANDLE m_handle;         /* the port id from OpenComm */
	DCB m_dcb;             /* the device control block */
#else
	int m_iFd;
	fd_set m_fdRead;
	struct timeval m_timeout;
#endif

	bool m_bIsOpened;

#ifdef _MSC_VER
	static void ErrorExit(LPTSTR lpszFunction);
#endif
	void _Refresh();
};

