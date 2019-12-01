#include"../include/fast_io_device.h"

namespace
{
inline errno_t lippincott_function()
{
	try
	{
		throw;
	}
	catch(std::system_error const& error)
	{
		if(error.code().category()==std::generic_category())
			return error.code().value();
		return EIO;
	}
	catch(...)
	{
		return EIO;
	}
}
}

extern "C"
{

void cxx_fast_io_release(void* d) noexcept
{
	fast_io::dynamic_stream dyn(d);
}

errno_t cxx_fast_io_writes(void* d,char const* begin,char const* end) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(d);
		writes(dyn,begin,end);
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_reads(char** readed,void* d,char* begin,char* end) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(d);
		*readed=reads(dyn,begin,end);
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_flush(void* d) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(d);
		flush(dyn);
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_print_size_t(void* d,std::size_t s) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(d);
		print(dyn,s);
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_print_ptrdiff_t(void* d,std::ptrdiff_t s) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(d);
		print(dyn,s);
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_print_double(void* d,double s) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(d);
		print(dyn,s);
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_print_c_str(void* d,char const* s) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(d);
		print(dyn,s);
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_acquire_stdout(void** out) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(std::in_place_type<fast_io::system_io_handle>,fast_io::native_stdout_number);
		*out=dyn.opaque_base_pointer();
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}
errno_t cxx_fast_io_acquire_stderr(void** out) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(std::in_place_type<fast_io::system_io_handle>,fast_io::native_stdout_number);
		*out=dyn.opaque_base_pointer();
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}
errno_t cxx_fast_io_acquire_stdin(void** out) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(std::in_place_type<fast_io::system_io_handle>,fast_io::native_stdin_number);
		*out=dyn.opaque_base_pointer();
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_acquire_file(void** out,char const* name,char const* mode) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(std::in_place_type<fast_io::system_file>,name,mode);
		*out=dyn.opaque_base_pointer();
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

errno_t cxx_fast_io_acquire_bufferred_file(void** out,char const* name,char const* mode) noexcept
{
	try
	{
		fast_io::dynamic_base dyn(std::in_place_type<fast_io::basic_iobuf<fast_io::system_file>>,name,mode);
		*out=dyn.opaque_base_pointer();
		return {};
	}
	catch(...)
	{
		return lippincott_function();
	}
}

}