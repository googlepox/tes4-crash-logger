#include "MemoryProbe.hpp"
#include <format>
#include "Dereference.hpp"

namespace CrashLogger::Stack
{

    bool GetStringForRTTIorPDB(void** object, std::string& buffer, HANDLE hProcess)
    {
        try
        {
            if (!object) return false;

            // Validate it's a readable pointer
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQueryEx(hProcess, object, &mbi, sizeof(mbi)))
                return false;

            if (mbi.State != MEM_COMMIT)
                return false;

            if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE)))
                return false;

            // Check alignment (objects must be aligned)
            if ((reinterpret_cast<uintptr_t>(object) & 0x3) != 0)
                return false;

            // Try to read the vtable pointer
            void* vtablePtr = *(void**)object;

            if (!vtablePtr) return false;

            // Check if vtable is in executable memory (where vtables live)
            if (!VirtualQueryEx(hProcess, vtablePtr, &mbi, sizeof(mbi)))
                return false;

            if (!(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_READONLY)))
                return false;

			if (auto name = PDB::GetClassNameFromRTTIorPDB((void*)object, hProcess); !name.empty())
			{
				if (name.find('+') != std::string::npos)
					return false;

				auto addr = reinterpret_cast<std::uintptr_t>(object);

				if (auto rtti = CrashLogger::PDB::GetClassNameFromRTTI(object, hProcess); !rtti.empty())
				{
					buffer += std::format("0x{:08X} ==> RTTI: {}", static_cast<std::uint32_t>(addr), rtti);
					return true;
				}

				if (auto sym = CrashLogger::PDB::GetClassNameFromPDB(object, hProcess); !sym.empty())
				{
					buffer += std::format("0x{:08X} ==> PDB: {}", static_cast<std::uint32_t>(addr), sym);
					return true;
				}
			}
            return false;
        }
        catch (...) { return false; }
    }

	bool GetRealStringForLabel(void** object, std::string& buffer, HANDLE hProcess)
	{
		std::string labelName, objectName, description;
		if (GetStringForClassLabel(object, labelName, objectName, description, hProcess))
		{
			buffer += std::format("0x{:08X} ==> ", *(UInt32*)object) + labelName + ": " + objectName + ": " + description;
			return true;
		}

		if (GetStringForRTTIorPDB(object, buffer, hProcess))
		{
			return true;
		}
		if (GetAsString(hProcess, object, labelName, description))
		{
			buffer += std::format("0x{:08X} ==> ", *(UInt32*)object) + labelName + ": " + '"' + description + '"';
			return true;
		}

		return false;
	}

	bool GetStringForLabel(void** object, std::string& buffer, HANDLE hProcess)
	{
		__try
		{
			return GetRealStringForLabel(object, buffer, hProcess);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}

	std::string GetLineForObject(void** object, UInt32 depth, HANDLE hProcess)
	{
		if (!object || depth == 0) return "";

		std::string buffer;

		for (UInt32 i = 0; i < depth && object; ++i)
		{
			if (GetStringForLabel(object, buffer, hProcess))
			{
				return buffer;
			}

			UInt32 deref = Dereference<UInt32>(object);
			if (deref == 0) break;

			buffer += std::format("0x{:08X} ==> ", deref);
			object = (void**)deref;
		}

		return "";
	}

}
