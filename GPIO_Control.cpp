#include "GPIO_Control.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#define _DarkLogD	printf
#define _DarkLogE	printf
#define _DarkLogW	printf

//Tinkerboard 3N 전용 핀맵
const DARK_GPIO_PIN_DEF g_gpio_pins[] = {
	{false, 0, 0},		//0
	{false, 0, 0}, 		//1: VCC_5V0
	{false, 0, 0}, 		//2: VCC_3V3
	{false, 0, 0}, 		//3: GND
	{true, 4, 18}, 		//4
	{true, 3, 9}, 		//5
	{true, 4, 19}, 		//6
	{true, 3, 10}, 		//7
	{true, 4, 20}, 		//8
	{false, 3, 11}, 	//9:	I2C-5 - SCL
	{true, 4, 21}, 		//10
	{false, 3, 12}, 	//11:	I2C-5 - SDA
	{true, 4, 22}, 		//12
	{true, 0xFFFF, 6}, 		//13: SARADC_VIN6
	{true, 0xFFFF, 7}, 		//14: SARADC_VIN7
};
uint32_t g_num_gpio_pins = 15;

GPIO_Control::GPIO_Control()
{
	m_szTemp = new char[DARK_MIDSTR_S];
	m_szTemp[0] = 0;
}

GPIO_Control::~GPIO_Control()
{
	if (m_szTemp != NULL) delete[] m_szTemp;
}

const char *GPIO_Control::getDevName(uint32_t nPin) {
	if (nPin >= g_num_gpio_pins) return NULL;
	DARK_SNPRINTF2(m_szTemp, DARK_MIDSTR_S, "/dev/gpiochip%d", g_gpio_pins[nPin].nChip);
	return m_szTemp;
}

bool GPIO_Control::Write(uint32_t nPin, int32_t iValue) {
    struct gpiohandle_request rq;
    struct gpiohandle_data data;
    int fd, ret;
	const char *dev_name = NULL;
    
	if (nPin >= g_num_gpio_pins) {
		_DarkLogE("GPIO_Control: Invalid pin number\n");
		return false;
	}
	if (!g_gpio_pins[nPin].bValid || g_gpio_pins[nPin].nChip >= 0xFFFF) {
		_DarkLogE("GPIO_Control: Target Pin[%d] is not valid\n", nPin);
		return false;
	}

	dev_name = getDevName(nPin);
    fd = open(dev_name, O_RDONLY);
    if (fd < 0) {
        _DarkLogE("GPIO_Control: Unabled to open %s: %s\n", dev_name, strerror(errno));
        return false;
    }
    rq.lineoffsets[0] = g_gpio_pins[nPin].nLine;
    rq.flags = GPIOHANDLE_REQUEST_OUTPUT;
    rq.lines = 1;
    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
    close(fd);
    if (ret == -1) {
        _DarkLogE("GPIO_Control: Unable to line handle from ioctl : %s\n", strerror(errno));
        return false;
    }
    data.values[0] = (uint8_t)iValue;
    ret = ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    if (ret == -1) {
        _DarkLogE("GPIO_Control: Unable to set line value using ioctl : %s\n", strerror(errno));
		close(rq.fd);
		return false;
    }
    close(rq.fd);
	return true;
}

bool GPIO_Control::Read(uint32_t nPin, int32_t &OUT_value) {
    struct gpiohandle_request rq;
    struct gpiohandle_data data;
    int fd, ret;
	const char *dev_name = NULL;    

	if (nPin >= g_num_gpio_pins) {
		_DarkLogE("GPIO_Control: Invalid pin number\n");
		return false;
	}
	if (!g_gpio_pins[nPin].bValid) {
		_DarkLogE("GPIO_Control: Target Pin is not valid\n");
		return false;
	}

	if (g_gpio_pins[nPin].nChip >= 0xFFFF) {		
		DARK_SNPRINTF2(m_szTemp, DARK_MIDSTR_S, "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw", g_gpio_pins[nPin].nLine);
		fd = open(m_szTemp, O_RDONLY);
		if (fd < 0) {
			_DarkLogE("GPIO_Control: Unabled to open %s: %s\n", m_szTemp, strerror(errno));
			return false;
		}
		ret = read(fd, m_szTemp, DARK_MIDSTR_S);
		close(fd);
		if (ret <= 0) {
			_DarkLogE("GPIO_Control: Unabled to read value %s: %d\n", m_szTemp, ret);
			return false;
		}
		m_szTemp[ret] = 0;
		OUT_value = atoi(m_szTemp);
		return true;
	}
	else {
		dev_name = getDevName(nPin);

		fd = open(dev_name, O_RDONLY);
		if (fd < 0) {
			_DarkLogE("GPIO_Control: Unabled to open %s: %s\n", dev_name, strerror(errno));
			return false;
		}
		rq.lineoffsets[0] = g_gpio_pins[nPin].nLine;
		rq.flags = GPIOHANDLE_REQUEST_INPUT;
		rq.lines = 1;
		ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
		close(fd);
		if (ret == -1) {
			_DarkLogE("GPIO_Control: Unable to get line handle from ioctl : %s\n", strerror(errno));
			return false;
		}
		ret = ioctl(rq.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
		if (ret == -1) {
			_DarkLogE("GPIO_Control: Unable to get line value using ioctl : %s\n", strerror(errno));
			close(rq.fd);
			return false;
		}

		OUT_value = data.values[0];
		close(rq.fd);
		
		return true;
	}
}
