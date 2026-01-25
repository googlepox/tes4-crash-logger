#include "CLUtilities.hpp"

std::string pcName;
std::string userName;

int CrashLoggerExceptionFilter(unsigned int code)
{
    return EXCEPTION_EXECUTE_HANDLER;
}