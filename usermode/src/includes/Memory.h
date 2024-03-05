#pragma once

class Memory
{
public:
	DWORD get_proc_id();
	std::uintptr_t get_module_base();
	HANDLE get_handle();

	template <typename T>
	inline T read(std::uintptr_t addr) const;
	
	template <typename T>
	inline bool write(std::uintptr_t addr, T value) const;

	void close_handle();

private:
	std::uintptr_t m_client;
	DWORD m_proc_id;
	HANDLE m_handle;
};

template <typename T>
inline T Memory::read(std::uintptr_t addr) const
{
	T buffer;
	ReadProcessMemory(this->m_handle, reinterpret_cast<LPCVOID>(addr), &buffer, sizeof(buffer), nullptr);
	return buffer;
}

template <typename T>
inline bool Memory::write(std::uintptr_t addr, T value) const
{
	if (WriteProcessMemory(this->m_handle, reinterpret_cast<LPVOID>(addr), &value, sizeof(value), nullptr) == NULL)
		return false;
	return true;
}
