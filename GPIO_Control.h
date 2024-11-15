#pragma once
#include "common_inc.h"
#include "Information_Parser.h"

typedef struct DARK_GPIO_PIN_DEF {
	bool bValid;
	uint32_t nChip;
	uint32_t nLine;
} DARK_GPIO_PIN_DEF;



class GPIO_Control
{
public:
	GPIO_Control();
	~GPIO_Control();

	bool Write(uint32_t nPin, int32_t iValue);
	bool Read(uint32_t nPin, int32_t &OUT_value);

protected:
	char *m_szTemp;
	const char *getDevName(uint32_t nPin);
};

