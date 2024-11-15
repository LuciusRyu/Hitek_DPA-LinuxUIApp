#pragma once
#include "common_inc.h"

class Dark_Transport
{
public:
	Dark_Transport();
	~Dark_Transport();

	virtual int32_t dark_read(uint8_t* pBuf, int32_t size) = 0;
	virtual int32_t dark_write(uint8_t* pBuf, int32_t size) = 0;
};

