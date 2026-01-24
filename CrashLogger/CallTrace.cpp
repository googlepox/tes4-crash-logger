#include "CrashLogger.hpp"
#include "ExceptionHandler.hpp"
#include <GameTasks.h>
#include <GameAPI.h>
#include <Script.h>
#include <GameObjects.h>


namespace CrashLogger::Playtime
{
	std::stringstream output;

	std::chrono::time_point<std::chrono::system_clock> gameStart;
	std::chrono::time_point<std::chrono::system_clock> gameEnd;

	extern void Init() { gameStart = std::chrono::system_clock::now(); }

	extern void Process(EXCEPTION_POINTERS* info)
	try
	{
		gameEnd = std::chrono::system_clock::now();
		output << std::format("Playtime: {:%T}\n", gameEnd - gameStart);
	}
	catch (...) { output << "Failed to log playtime." << '\n'; }

	extern std::stringstream& Get() { output.flush(); return output; }

}


namespace CrashLogger::GameData
{
	UInt16 textureCounters[8] = { 0 };

	// Oblivion internal globals
	NiTexture** g_NiTextureListHead =
		reinterpret_cast<NiTexture**>(0x00B3F700);

	NiTexture** g_NiTextureListTail =
		reinterpret_cast<NiTexture**>(0x00B3F704);

	CRITICAL_SECTION* g_NiTextureCS =
		reinterpret_cast<CRITICAL_SECTION*>(0x00B3F780);

	static void __fastcall AddTexture(UInt32 size)
	{
		if (size <= 128)
			textureCounters[0]++;
		else if (size <= 256)
			textureCounters[1]++;
		else if (size <= 512)
			textureCounters[2]++;
		else if (size <= 1024)
			textureCounters[3]++;
		else if (size <= 2048)
			textureCounters[4]++;
		else if (size <= 4096)
			textureCounters[5]++;
		else if (size <= 8192)
			textureCounters[6]++;
	}

	static void TextureCounter()
	{
		NiTexture* head = *g_NiTextureListHead;
		while (head)
		{
			UInt32 h = head->GetHeight();
			UInt32 w = head->GetWidth();
			AddTexture(h > w ? h : w);
			textureCounters[7]++;
			head = head->nextTex;
		}
	}

	extern void __fastcall Process(EXCEPTION_POINTERS* info)
	{
		try
		{
			memset(textureCounters, 0, sizeof(textureCounters));
			TextureCounter();
			_MESSAGE("\nPlayer Data:");
			try
			{
				_MESSAGE("    ");
				PlayerCharacter* pPlayer = (*g_thePlayer);
				if (pPlayer)
				{
					TESObjectCELL* pParentCell = pPlayer->parentCell;
					if (pParentCell)
						_MESSAGE("Cell:       %08X (\"%s\")", pParentCell->refID, pParentCell->GetEditorID());
					else
						_MESSAGE("Cell:       None");

					TESWorldSpace* pWorld = pPlayer->parentCell->worldSpace;
					if (pWorld)
						_MESSAGE("World:      %08X (\"%s\")", pWorld->refID, pWorld->GetEditorID());
					else
						_MESSAGE("World:      None");

					//const NiPoint3& rPosition = pPlayer->kPosition;
					//const NiPoint3& rRotation = pPlayer->kRotation;
					//_MESSAGE("Position:   X: %.2f Y: %.2f Z: %.2f", rPosition.x, rPosition.y, rPosition.z);
					//_MESSAGE("Rotation:   X: %.2f Y: %.2f Z: %.2f", rRotation.x, rRotation.y, rRotation.z);
				}
				else
				{
					_MESSAGE("Failed to get player character.");
				}
			}
			catch (...)
			{
				_MESSAGE(" Failed to get player character info.");
			}

			TextureCounter();

			//UInt32 uiFaceGenMeshes = 0;
			//if (BSFaceGenManager::GetSingleton() && BSFaceGenManager::GetSingleton()->pModelMap)
				//uiFaceGenMeshes = BSFaceGenManager::GetSingleton()->pModelMap->kEntryMap.GetCount();

			UInt32 uiModels = 0;
			UInt32 uiAnimations = 0;
			if (ModelLoader::GetSingleton() && IsReadablePtr(ModelLoader::GetSingleton()))
			{
				if (ModelLoader::GetSingleton()->modelMap)
					uiModels = ModelLoader::GetSingleton()->modelMap->GetUnk0CCount();
				if (ModelLoader::GetSingleton()->kfMap)
					uiAnimations = ModelLoader::GetSingleton()->kfMap->GetUnk0CCount();
			}
			_MESSAGE("\nScript Info:");
			try
			{
				ScriptRunner* pRunner = ScriptRunner::GetSingleton();

				if (pRunner && IsReadablePtr(pRunner))
				{
					Script* curScript = pRunner->script;

					if (curScript && curScript->data)
					{
						_MESSAGE("\nCurrently running script:    ");
						if (curScript)
						{
							std::string_view strEDID = curScript->GetEditorName();
							if (curScript->flags & TESForm::FormFlags::kFormFlags_Temporary)
							{
								if (strEDID.empty())
									_MESSAGE("Script:	   Temporary Script");
								else
									_MESSAGE("Script:	   Temporary Script (\"%s\")", strEDID.data());
							}
							else
							{
								if (strEDID.empty())
									_MESSAGE("Script:	   %08X", curScript->refID);
								else
									_MESSAGE("Script:	   %08X (\"%s\")", curScript->refID, strEDID.data());
							}

						}
						else
							_MESSAGE("Script:	   None");

						/*if (pRunner->pCurrentContainer)
							_MESSAGE("Container:    %08X (\"%s\")", pRunner->pCurrentContainer->refID, pRunner->pCurrentContainer->GetEditorID());
						else
							_MESSAGE("Container:    None");

						if (pRunner->pCurrentObject)
							_MESSAGE("Object:	   %08X (\"%s\")", pRunner->pCurrentObject->refID, pRunner->pCurrentObject->GetEditorID());
						else
							_MESSAGE("Object:	   None");

						_MESSAGE("Error Code:   %u", pRunner->eError); */
					}
				}
			}
			catch (...)
			{
				_MESSAGE(" Failed to get currently running script info.");
			}

			_MESSAGE("\nLoaded assets:");
			{
				_MESSAGE("    ");
				_MESSAGE("Textures:       %i", textureCounters[7]);
				_MESSAGE("  <= 128:  %i", textureCounters[0]);
				_MESSAGE("  <= 256:  %i", textureCounters[1]);
				_MESSAGE("  <= 512:  %i", textureCounters[2]);
				_MESSAGE("  <= 1024: %i", textureCounters[3]);
				_MESSAGE("  <= 2048: %i", textureCounters[4]);
				_MESSAGE("  <= 4096: %i", textureCounters[5]);
				_MESSAGE("  <= 8192: %i", textureCounters[6]);
				_MESSAGE("Models:         %i", uiModels);
				//_MESSAGE("FaceGen Models: %i", uiFaceGenMeshes);
				_MESSAGE("Animations:     %i", uiAnimations);
			}

			/*_MESSAGE("\nProcess Lists:");
			uint32_t uiTotal = 0;
			{
				_MESSAGE("    ");
				for (uint32_t i = 0; i < 4; i++)
				{
					uint32_t uiStart = ProcessLists::GetSingleton()->kAllProcessArrays.uiBeginOffsets[i];
					uint32_t uiEnd = ProcessLists::GetSingleton()->kAllProcessArrays.uiEndOffsets[i];
					uint32_t uiCount = uiEnd - uiStart;
					const char* pProcessName = "Invalid";
					switch (i)
					{
					case BaseProcess::kProcessLevel_High:
						pProcessName = "High";
						break;
					case BaseProcess::kProcessLevel_MiddleHigh:
						pProcessName = "Middle High";
						break;
					case BaseProcess::kProcessLevel_MiddleLow:
						pProcessName = "Middle Low";
						break;
					case BaseProcess::kProcessLevel_Low:
						pProcessName = "Low";
						break;
					default:
						__assume(0);
					}
					_MESSAGE("%s: %i", pProcessName, uiCount);
					uiTotal += uiCount;
				}
			}
			_MESSAGE("Total: %i", uiTotal); */
		}
		catch (...)
		{
			_MESSAGE("Failed to print game stats.\n");
		}
	}
}

namespace CrashLogger::Exception
{

	std::stringstream output;

	extern void Process(EXCEPTION_POINTERS* info)
	{
		try
		{
			output << std::format("Exception: {} ({:08X})\n", GetExceptionAsString(info->ExceptionRecord->ExceptionCode), info->ExceptionRecord->ExceptionCode);
			if (GetLastError()) output << std::format("Last Error: {} ({:08X})\n", SanitizeString(GetErrorAsString(GetLastError())), GetLastError());
		}
		catch (...) { output << "Failed to log exception." << '\n'; }
	}

	extern std::stringstream& Get() { output.flush(); return output; }
}

namespace CrashLogger::Thread
{
	std::stringstream output;

	std::string GetThreadName()
	{
		std::string threadName;
		wchar_t* pThreadName = NULL;
		HRESULT hr = GetThreadDescription(GetCurrentThread(), &pThreadName);
		std::wstring wThreadName(pThreadName);
		std::transform(wThreadName.begin(), wThreadName.end(), std::back_inserter(threadName), [](wchar_t c) { return (char)c; });
		LocalFree(pThreadName);
		return threadName;
	}

	extern void Process(EXCEPTION_POINTERS* info)
		try { output << "Thread: " << GetThreadName() << '\n'; }
	catch (...) { output << "Failed to log thread name." << '\n'; }

	extern std::stringstream& Get() { output.flush(); return output; }
}

namespace CrashLogger::Calltrace
{
	std::stringstream output;

	std::string GetCalltraceFunction(UInt32 eip, UInt32 ebp, HANDLE process)
	{
		/*if (GetModuleFileName((HMODULE)frame.AddrPC.Offset, path, MAX_PATH)) {  //Do this work on non base addresses even on  Windows? Cal directly the LDR function?
		if (!SymLoadModule(process, NULL, path, NULL, 0, 0)) Log() << FormatString("Porcoddio %0X", GetLastError());
		}*/

		const auto moduleBase = PDB::GetModuleBase(eip, process);

		std::string begin = std::format("0x{:08X} | ", ebp);

		std::string middle;

		const auto moduleOffset = (moduleBase != 0x00400000) ? eip - moduleBase + 0x10000000 : eip;

		if (const auto module = PDB::GetModule(eip, process); module.empty())
			middle = std::format("{:>28s} (0x{:08X}) | {:<40s} |", "???", moduleOffset, "(Corrupt stack or heap?)");
		else if (const auto symbol = PDB::GetSymbol(eip, process); symbol.empty())
			middle = std::format("{:>28s} (0x{:08X}) | {:<40s} |", module, moduleOffset, "");
		else
			middle = std::format("{:>28s} (0x{:08X}) | {:<40s} |", module, moduleOffset, symbol);

		std::string end;

		if (const auto line = PDB::GetLine(eip, process); !line.empty())
		{
			end = " " + line;
		}

		return begin + middle + end;
	}

	extern void Process(EXCEPTION_POINTERS* info)
	{
		try {
			HANDLE process = GetCurrentProcess();
			HANDLE thread = GetCurrentThread();

			DWORD machine = IMAGE_FILE_MACHINE_I386;
			CONTEXT context = {};
			memcpy(&context, info->ContextRecord, sizeof(CONTEXT));

			SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS);

			char workingDirectory[MAX_PATH];
			char symbolPath[MAX_PATH];
			char altSymbolPath[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, workingDirectory);
			GetEnvironmentVariable("_NT_SYMBOL_PATH", symbolPath, MAX_PATH);
			GetEnvironmentVariable("_NT_ALTERNATE_SYMBOL_PATH", altSymbolPath, MAX_PATH);
			std::string lookPath = std::format("{};{}\\Data\\OBSE\\plugins;{};{}", workingDirectory, workingDirectory, symbolPath, altSymbolPath);

			//	SymSetExtendedOption((IMAGEHLP_EXTENDED_OPTIONS)SYMOPT_EX_WINE_NATIVE_MODULES, TRUE);
			if (!SymInitialize(process, lookPath.c_str(), true)) {
				//output << "Error initializing symbol store" << '\n';
			}
				

			//	SymSetExtendedOption((IMAGEHLP_EXTENDED_OPTIONS)SYMOPT_EX_WINE_NATIVE_MODULES, TRUE);

			STACKFRAME frame = {};
			frame.AddrPC.Offset = info->ContextRecord->Eip;
			frame.AddrPC.Mode = AddrModeFlat;
			frame.AddrFrame.Offset = info->ContextRecord->Ebp;
			frame.AddrFrame.Mode = AddrModeFlat;
			frame.AddrStack.Offset = info->ContextRecord->Esp;
			frame.AddrStack.Mode = AddrModeFlat;
			DWORD eip = 0;
			
			// crutch to try to copy dbghelp before.
			output << "Calltrace:" << '\n' << std::format("{:^10} |  {:^40} | {:^40} | Source", "ebp", "Function Address", "Function Name") <<
				'\n';

			while (StackWalk(machine, process, thread, &frame, &context, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL)) {
				/*
				Using  a PDB for OBSE from VS2019 is causing the frame to repeat, but apparently only if WINEDEBUG=+dbghelp isn't setted. Is this a wine issue?
				When this happen winedbg show only the first line (this happens with the first frame only probably, even if there are more frames shown when using WINEDEBUG=+dbghelp )
				*/
				if (frame.AddrPC.Offset == eip) break;
				eip = frame.AddrPC.Offset;
				output << GetCalltraceFunction(frame.AddrPC.Offset, frame.AddrFrame.Offset, process) << '\n';
			}
		}
		catch (...) { output << "Failed to log callstack." << '\n'; }
	}

	extern std::stringstream& Get() { output.flush(); return output; }
}