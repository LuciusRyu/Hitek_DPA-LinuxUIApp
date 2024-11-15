#include "UART_Serial.h"

#ifndef _MSC_VER
#	include <sys/wait.h>
#	include <sys/select.h>
#	include <signal.h>

#	include <termios.h>
#	include <fcntl.h> // Contains file controls like O_RDWR
#	include <sys/ioctl.h>
#endif

#define _DarkLogD(fmt, args...)	printf("[UART_DEBUG]: " fmt, ##args)
#define _DarkLogE(fmt, args...)	printf("[UART_ERROR]: " fmt, ##args)
#define _DarkLogW(fmt, args...)	printf("[UART_WARNING]: " fmt, ##args)

#ifdef _MSC_VER
WCHAR* ConvertUTF8ToWCHAR(const char* szUTF8, WCHAR* OUT_pTar)
{
	int nLength;
	static WCHAR wsStr[1024];
	WCHAR* pwStr;

	if (OUT_pTar == NULL) pwStr = wsStr;
	else pwStr = OUT_pTar;

	nLength = MultiByteToWideChar(CP_UTF8, 0, szUTF8, lstrlenA(szUTF8) + 1, NULL, NULL);
	if (nLength > 1024) return NULL;

	MultiByteToWideChar(CP_UTF8, 0, szUTF8, lstrlenA(szUTF8) + 1, pwStr, nLength);

	return pwStr;
}
#endif

UART_Serial::UART_Serial() {
#ifdef _MSC_VER
	m_handle = NULL;         
	memset(&m_dcb, 0, sizeof(DCB));            
#else
	m_iFd = 0;
#endif

	m_bIsOpened = false;
}

UART_Serial::~UART_Serial() {
	Close();
}

void UART_Serial::_Refresh() {
	if (!m_bIsOpened) return;

#ifdef _MSC_VER
	PurgeComm(m_handle, PURGE_TXCLEAR | PURGE_RXCLEAR);
#else
	tcflush(m_iFd, TCIFLUSH);

	//Clear DTR --> �̷� ���� �̰� ���ϴϱ� ���̳ʸ� ����� �� �ȵǳ�..
	uint32_t dtr_bit = TIOCM_DTR;
	ioctl(m_iFd, TIOCMBIC, &dtr_bit);
#endif
}

int32_t UART_Serial::Open(void* pPort, int32_t iBaudRate, char cParity, int32_t iDataBit, int32_t iStopBit)
{
#ifdef _MSC_VER
	COMMTIMEOUTS timeouts;

	WCHAR* wPort = ConvertUTF8ToWCHAR((char*)pPort, NULL);

	if ((m_handle = CreateFile(wPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
	{
		//printf("---------------------- INVALID HANDEL\n");
		return -1;
	}

	if (SetupComm(m_handle, DARK_MAXPATH, DARK_MAXPATH) != TRUE) {
		CloseHandle(m_handle);
		//printf("---------------------- Setup comm\n");
		return -2;
	}

	GetCommTimeouts(m_handle, &timeouts);
	timeouts.ReadTotalTimeoutConstant = 10;
	timeouts.ReadTotalTimeoutMultiplier = 5;
	SetCommTimeouts(m_handle, &timeouts);

	//printf("baud=%ld parity=%d data=%d stop=%d\n", iBaudRate, cParity, iDataBit, iStopBit);
	memset(&m_dcb, 0, sizeof(DCB));
	GetCommState(m_handle, &m_dcb);
	m_dcb.DCBlength = sizeof(DCB);
	m_dcb.BaudRate = iBaudRate;
	m_dcb.Parity = cParity;
	m_dcb.fBinary = 1;
	m_dcb.ByteSize = iDataBit;
	m_dcb.StopBits = iStopBit;

	if (SetCommState(m_handle, &m_dcb) != TRUE) {
		//printf("---------------------- SetCommState: %d\n", GetLastError());
		CloseHandle(m_handle);
		return -4;
	}

	m_bIsOpened = true;
	return 0;
#else
	if (!(cParity <= 0 && iDataBit == 8 && iStopBit <= 0)) {
		_DarkLogE("Error - Oen - Only binary mode support....\n");
		return -1;
	}

	struct termios tty;
	char* szName = (char*)pPort;

	m_iFd = open(szName, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	if (m_iFd <= 0) return -2;

	//tcgetattr(m_iFd, &tty); //�̰� ���߿� ������ ���ؼ� �ϴ°ǵ� ���� ���� ���� �Ŵϱ�~
	bzero(&tty, sizeof(struct termios));

	int32_t iSpd = 0;
	switch (iBaudRate)
	{
	default:
	case 115200: iSpd = B115200; break;
	case 57600: iSpd = B57600; break;
	case 38400: iSpd = B38400; break;
	case 19200: iSpd = B19200; break;
	case 9600: iSpd = B9600; break;
	case 4800: iSpd = B4800; break;
	case 2400: iSpd = B2400; break;
	}

	tty.c_cflag = CLOCAL | CREAD;
	cfsetispeed(&tty, iSpd);
	cfsetospeed(&tty, iSpd);

	tty.c_cflag &= ~PARENB; //no checkbit
	tty.c_cflag &= ~CSTOPB; //1bit stop bit
	tty.c_cflag &= ~CRTSCTS; //no flow control

	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8; /* Select 8 data bits */

	tty.c_cflag &= ~CRTSCTS; // no hw flow control
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // no sw flow control

	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw input mode   
													// raw output mode   
	tty.c_oflag &= ~OPOST;

	if (tcsetattr(m_iFd, TCSANOW, &tty)) {
		//WriteSystemLog(DARK_LOGLEVEL_ERROR, "UART_METHOD", "OpenBinary - Set Attribute failed");
		return -3;
	}

	tcflush(m_iFd, TCIFLUSH);
	if (fcntl(m_iFd, F_SETFL, O_NONBLOCK))
	{
		//WriteSystemLog(DARK_LOGLEVEL_ERROR, "UART_METHOD", "fcntl - FNDELAY failed");
		return -4;
	}

	//Clear DTR --> �̷� ���� �̰� ���ϴϱ� ���̳ʸ� ����� �� �ȵǳ�..
	uint32_t dtr_bit = TIOCM_DTR;
	ioctl(m_iFd, TIOCMBIC, &dtr_bit);

	FD_ZERO(&m_fdRead);
	FD_SET(m_iFd, &m_fdRead);

	m_bIsOpened = true;
	return 0;
#endif
}

int32_t UART_Serial::Close()
{
	if (!m_bIsOpened) return -1;
	_Refresh();
#ifdef _MSC_VER
	CloseHandle(m_handle);
#else
	close(m_iFd);
	m_iFd = 0;
#endif

	m_bIsOpened = false;
	return 1;
}

int32_t UART_Serial::dark_read(uint8_t* pBuf, int32_t size)
{
	if (m_bIsOpened != true) return -1;
#ifdef _MSC_VER
	DWORD stCode, dwR;
	COMSTAT cs;

	ClearCommError(m_handle, &stCode, &cs);
	if (cs.cbInQue > 0) {
		if (ReadFile(m_handle, pBuf, size > cs.cbInQue ? cs.cbInQue : size, &dwR, NULL) == TRUE) return (int32_t)dwR;
	}
	return 0;
#else
	int32_t iRes;

	FD_ZERO(&m_fdRead);

	m_timeout.tv_sec = 0;
	m_timeout.tv_usec = 100000;
	FD_SET(m_iFd, &m_fdRead);

	iRes = select(m_iFd + 1, &m_fdRead, NULL, NULL, &(m_timeout));
	if (iRes < 0) {
		_DarkLogW("UART_Serial - dark_read - native_read function - select failed\n");
		//return iRes;
		return 0;
	}
	if (iRes == 0) return 0;

	if (FD_ISSET(m_iFd, &m_fdRead)) {
		iRes = read(m_iFd, pBuf, size);
		//_DarkLogD("UART_Serial - dark_read - read=%d\n", iRes);
		return iRes;
	}
	return 0;
#endif
}

int32_t UART_Serial::dark_write(uint8_t* pBuf, int32_t size)
{
	if (m_bIsOpened != true) return -1;

#ifdef _MSC_VER
	DWORD stCode, dwW;
	COMSTAT cs;

	ClearCommError(m_handle, &stCode, &cs);
	if (size >= (DARK_MAXPATH - cs.cbOutQue)) {
		//printf("Serial write failed, Q: %d / %d\n", size, (DARK_UART_MAX_PACKET - cs.cbOutQue));
		return 0;
	}
	if (WriteFile(m_handle, pBuf, size, &dwW, NULL) != TRUE) {
		//printf("Serial write failed, %d / %d\n", size, dwW);
		return 0;
	}
	//printf("native write %d\n", size);
	//for (int i = 0; i < size; i++) printf("%02X ", pBuf[i]);
	//printf("\n");
	return (int32_t)dwW;
#else
	int32_t iRes;

	iRes = write(m_iFd, pBuf, size);
	//_DarkLogD("UART - Total %d/%d bytes writed\n", iRes, size);
	return iRes;
#endif
}
