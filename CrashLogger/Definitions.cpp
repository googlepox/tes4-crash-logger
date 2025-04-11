#include "Definitions.hpp"
#include "CrashLogger.hpp"

#define INIT_MODULE(mod) namespace mod { extern void Init(); }

INIT_MODULE(CrashLogger)

void Inits()
{
	CrashLogger::Init();
}