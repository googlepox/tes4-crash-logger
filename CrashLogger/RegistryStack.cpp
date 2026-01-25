#include "CrashLogger.hpp"
#include <ranges>
#include <iostream>
#include <ios>
#include <istream>
#include "CrashLoggerAPI.h"
#include "StackAPI.hpp"

namespace CrashLogger::Registry
{
	std::stringstream output;

	extern void Process(EXCEPTION_POINTERS* info)
	{
		try
		{
			output << "Registry:" << '\n'
				<< std::format("REG | {:^10} | DEREFERENCE INFO", "Value") << '\n';

			const std::map<std::string, UInt32> registers{
				{ "eax", info->ContextRecord->Eax },
				{ "ebx", info->ContextRecord->Ebx },
				{ "ecx", info->ContextRecord->Ecx },
				{ "edx", info->ContextRecord->Edx },
				{ "edi", info->ContextRecord->Edi },
				{ "esi", info->ContextRecord->Esi },
				{ "ebp", info->ContextRecord->Ebp },
				{ "esp", info->ContextRecord->Esp },
				{ "eip", info->ContextRecord->Eip },
			};

			for (const auto& [name, value] : registers)
			{
				std::stringstream str;
				str << std::format("{} | 0x{:08X} | ", name, value);
				std::string buffer = Stack::GetLineForObject((void**)value, 5, GetCurrentProcess());
				if (!buffer.empty() && buffer != "") {
					str << buffer;
				}
				output << str.str() << '\n';
			}
		}
		catch (...) { output << "Failed to log registry." << '\n'; }
	}

	extern std::stringstream& Get() { output.flush(); return output; }
}

namespace CrashLogger::Stack
{
	std::map<UInt32, UInt8> memoize;

	std::stringstream output;

	UInt32 GetESPi(UInt32* esp, UInt32 i) try { return esp[i]; }
	catch (...) { return 0; }

	extern void Process(EXCEPTION_POINTERS* info)
		try {
		output << "Stack:" << '\n' << std::format("  # | {:^10} | DEREFERENCE INFO", "Value") << '\n';
		const auto esp = reinterpret_cast<UInt32*>(info->ContextRecord->Esp);
		for (unsigned int i : std::views::iota(0x0, 0x100)) {
			const auto espi = GetESPi(esp, i);
			const auto str = Stack::GetLineForObject((void**)espi, 5, GetCurrentProcess());
			if (i <= 0x8 || (!str.empty() && !memoize.contains(espi)))
			{
				std::stringstream line;
				line << std::format(" {:2X} | 0x{:08X} | ", i, espi);
				if (!memoize.contains(espi))
				{
					if (!str.empty()) line << str;
					memoize.emplace(espi, i);
				}
				else
				{
					line << std::format("Identical to {:2X}", memoize[espi]);
				}
				output << line.str() << '\n';

			}
		}
	}
	catch (...) { output << "Failed to log stack." << '\n'; }

	extern std::stringstream& Get() { output.flush(); return output; }
}
