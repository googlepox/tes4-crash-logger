#pragma once
#include "CLUtilities.hpp"
#include <format>
#include <string>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <vector>
#include "Formatter.hpp"
#include "Dereference.hpp"
#include "PDB.h"

namespace CrashLogger::Playtime { inline void Init(); inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Exception { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Thread { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Calltrace { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Registry { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Stack { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Modules { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Install { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Memory
{
	inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get();
	bool InstallAllocHook(); bool InstallFreeHook();  void StartMemoryProfiler(); void LaunchHelper();
}
//namespace CrashLogger::Mods { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::Device { inline void Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }
namespace CrashLogger::GameData { inline void __fastcall Process(EXCEPTION_POINTERS* info); inline std::stringstream& Get(); }


namespace CrashLogger::Labels
{
	inline std::string AsUInt32(void* ptr) { return std::format("{:#08X}", **static_cast<UInt32**>(ptr)); }

	template<typename T> std::string As(void* ptr)
	{
		try {
			if (auto sanitized = Dereference<T>((UInt32)ptr))
			{
				return LogClassLineByLine(*sanitized);
			}
			else {
				return "Unable to dereference";
			}
		}
		catch (...)
		{
			return "Failed to format";
		}
	}

	class Label
	{
		static inline std::vector<std::unique_ptr<Label>> labels;
		typedef std::string(*FormattingHandler)(void* ptr);
		static inline FormattingHandler lastHandler = nullptr;

	public:

		UInt32 address;
		UInt32 size;
		FormattingHandler function = lastHandler;
		std::string name;

		static auto& GetAll() { return labels; }

		Label() : address(0), size(0), function(nullptr) {}
		virtual ~Label() = default;

		Label(UInt32 aAddress, FormattingHandler aFunction = lastHandler, std::string aName = "", UInt32 aSize = 4)
			: Label()
		{
			address = aAddress;
			size = aSize;
			function = aFunction;
			name = std::move(aName);

			lastHandler = aFunction;
		};


		bool Satisfies(void* ptr) const
		{
			__try {
				return *static_cast<UInt32*>(ptr) >= address && *static_cast<UInt32*>(ptr) <= address + size;
			}
			__except (CrashLoggerExceptionFilter(GetExceptionCode()))
			{
				return false;
			}
		}

		static std::string GetTypeName(void* ptr, HANDLE hProcess)
		{
			return CrashLogger::PDB::GetClassNameFromRTTIorPDB(ptr, hProcess);
		}

		virtual std::string GetLabelName() const { return "None"; }

		virtual std::string GetName(void* object, HANDLE hProcess) const { return name; }

		virtual std::string GetDescription(void* object) const
		{
			if (function) return function(object);
			return "";
		}
	};

	class LabelClass : public Label
	{
	public:
		using Label::Label;

		std::string GetLabelName() const override { return "Class"; }

		std::string GetName(void* object, HANDLE hProcess) const override { return name.empty() ? GetTypeName(object, hProcess) : name; }
	};

	class LabelGlobal : public Label
	{
	public:
		using Label::Label;

		std::string GetLabelName() const override { return "Global"; }
	};

	class LabelEmpty : public Label
	{
	public:
		using Label::Label;
	};

	template <class LabelType = LabelClass, class... _Types> void Push(_Types... args) {
		Label::GetAll().push_back(std::make_unique<LabelType>(std::forward<_Types>(args)...));
	}

#ifdef CRASHLOGGER_INCLUDE_OBSE
	void FillOBSELabels();
#endif

	inline void FillLabels()
	{
#ifdef CRASHLOGGER_INCLUDE_OBSE
		FillOBSELabels();
#endif
	}
}
