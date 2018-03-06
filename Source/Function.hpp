#pragma once

template <typename T> class Function
{
public:
	using result_type = typename std::function<T>::result_type;

	Function() : func(reinterpret_cast<T*>(nullptr)) {};
	Function(const std::uintptr_t& address) : func(reinterpret_cast<T*>(address)) {}
	Function(const T*& function) : func(function) {}
	Function(const std::function<T>& function) : func(function) {}
	Function(const Function<T>& function) : func(function.func) {}

	Function& operator=(const std::uintptr_t& address)
	{
		this->func = std::function<T>(reinterpret_cast<T*>(address));
		return *this;
	}
	Function& operator=(const T*& function)
	{
		this->func = function;
		return *this;
	}
	Function& operator=(const std::function<T>& function)
	{
		this->func = function;
		return *this;
	}
	Function& operator=(const Function<T>& function)
	{
		this->func = function.func;
		return *this;
	}

    operator bool()
    {
        return func.operator bool();
    }

	template <typename ...Args> result_type operator()(Args&& ...args)
	{
		return func(args...);
	}
	result_type operator()(void)
	{
		return func();
	}

protected:
	std::function<T> func;

};
