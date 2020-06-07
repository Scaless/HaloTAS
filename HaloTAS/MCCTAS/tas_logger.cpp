#include "tas_logger.h"

void tas_logger::log(const wchar_t* format, ...)
{
	va_list args;
	va_start(args, format);
	wchar_t buffer[2048];
	vswprintf_s(buffer, format, args);
	wprintf(buffer);
	va_end(args);
}


