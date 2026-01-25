#pragma once
namespace CrashLogger
{

	class SE_Exception
	{
	public:
		SE_Exception() {}
		~SE_Exception() {}
	};

	template<typename T>
	class Dereference
	{
		intptr_t pointer;
		std::size_t size;

	public:
		Dereference(intptr_t pointer, std::size_t size) : pointer(pointer), size(size) {}

		Dereference(intptr_t pointer) : pointer(pointer), size(sizeof(T)) {}
		Dereference(const void* pointer) : pointer((intptr_t)pointer), size(sizeof(T)) {}

		operator bool()
		{
			return IsValidPointer();
		}

		operator T* ()
		{
			if (IsValidPointer())
			{
				return reinterpret_cast<T*>(pointer);
			}
			return nullptr;

			//throw std::runtime_error("Bad dereference");
		}

		T* operator->()
		{
			if (IsValidPointer())
			{
				return reinterpret_cast<T*>(pointer);
			}
			return nullptr;
			//throw std::runtime_error("Bad dereference");
		}

	private:
		bool IsValidAddress() const
		{
			MEMORY_BASIC_INFORMATION mbi;
			if (::VirtualQuery((void*)pointer, &mbi, sizeof(mbi)))
			{
				if (mbi.State != MEM_COMMIT) return false;

				DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
				if ((mbi.Protect & mask) == 0) return false;

				if (mbi.Protect & PAGE_GUARD) return false;
				if (mbi.Protect & PAGE_NOACCESS) return false;

				if (size_t(mbi.RegionSize) < size) return false;

				return true;
			}
			return false;
		}

		bool AttemptDereference() const
			try
		{
			// Attempt to read the address as a UInt32
			volatile UInt32 temp = *reinterpret_cast<const volatile UInt32*>(pointer);
			return true;
		}
		catch (...)
		{
			return false;
		}

		bool IsVtableValid() const
		{
			//			if (vtables_.find(vtable) == vtables_.end()) return false;

			UInt32 vtable = *reinterpret_cast<UInt32*>(pointer);
			if (vtable > 0xA283D0 && vtable < 0xAB0000)
				return true;

			return false;
		}

		bool IsValidPointer() const
			try
		{
			if (!IsValidAddress()) return false;

			if (!AttemptDereference()) return false;

			if (!IsVtableValid()) return false;

			return true;
		}
		catch (...)
		{
			return false;
		}
	};
}