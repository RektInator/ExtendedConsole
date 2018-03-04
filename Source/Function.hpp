#pragma once

template <typename T> class Function
{
public:
	using result_type = typename std::function<T>::result_type;

	Function() : m_func(reinterpret_cast<T*>(nullptr)) {};
	Function(const std::uintptr_t& address) : m_func(reinterpret_cast<T*>(address)) {}
	Function(const std::function<T>& function) : m_func(function) {}
	Function(const Function<T>& function) : m_func(function.m_func) {}

	Function& operator=(const std::uintptr_t& address)
	{
		this->m_func = std::function<T>(reinterpret_cast<T*>(address));
		return *this;
	}
	Function& operator=(const std::function<T>& function)
	{
		this->m_func = function;
		return *this;
	}
	Function& operator=(const Function<T>& function)
	{
		this->m_func = function.m_func;
		return *this;
	}

	template <typename ...Args> result_type operator()(Args&& ...args)
	{
		return m_func(args...);
	}
	result_type operator()(void)
	{
		return m_func();
	}

protected:
	std::function<T> m_func;

};
