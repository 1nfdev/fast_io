#pragma once

#include"fast_io_hosted.h"
//This header file defines in/out/err/log like traditional C/C++ does
//out and err will direct write to system call.
//The magic here is that data will first write to std::u8string and then write out
//There is no way to implement in without a buffer. input must have a buffer.

namespace fast_io
{
inline namespace
{

using in_type = fast_io::basic_iomutex<fast_io::basic_ibuf<native_io_handle>>;

in_type in(native_stdin_number);
inline native_io_handle out(native_stdout_number);
inline native_io_handle err(native_stderr_number);

using log_type = basic_obuf<native_io_handle>;

inline auto out_buf()
{
	return log_type(native_stdout_number);
}

inline auto log()
{
	return log_type(native_stderr_number);
}

}
}

/*
I know putting global would be a problem. However, I think put some special cases to global would reduce abusing of using namespace

print is more nature and teachable for new C++ programmmers and most languages on the world (basic, lua, python) are all doing this

Also, it allows you for copying some other languages' into C++
*/

template<typename T,typename... Args>
requires (!fast_io::output_stream<T>)&&requires(T&& t,Args&& ...args)
{
	print(fast_io::out,std::forward<T>(t),std::forward<Args>(args)...);
}
inline constexpr void print(T&& t,Args&& ...args)
{
	print(fast_io::out,std::forward<T>(t),std::forward<Args>(args)...);
}

template<typename T,typename... Args>
requires (!fast_io::output_stream<T>)&&requires(T&& t,Args&& ...args)
{
	println(fast_io::out,std::forward<T>(t),std::forward<Args>(args)...);
}
inline constexpr void println(T&& t,Args&& ...args)
{
	println(fast_io::out,std::forward<T>(t),std::forward<Args>(args)...);
}

template<typename... Args>
requires requires(Args&& ...args)
{
	fprint(fast_io::out,std::forward<Args>(args)...);
}
inline constexpr void fprint(Args&& ...args)
{
	fprint(fast_io::out,std::forward<Args>(args)...);
}