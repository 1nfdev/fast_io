#pragma once

#include<iostream>

namespace fast_io
{

namespace details
{

inline constexpr std::ios::openmode calculate_fstream_file_open_mode(open_mode om)
{
	std::ios::openmode ios_om{};
	if((om&open_mode::app)!=open_mode::none)
		ios_om=ios_om|std::ios::app;
	if((om&open_mode::binary)!=open_mode::none)
		ios_om=ios_om|std::ios::binary;
	if((om&open_mode::in)!=open_mode::none)
		ios_om=ios_om|std::ios::in;
	if((om&open_mode::out)!=open_mode::none)
		ios_om=ios_om|std::ios::out;
	if((om&open_mode::trunc)!=open_mode::none)
		ios_om=ios_om|std::ios::trunc;
	if((om&open_mode::ate)!=open_mode::none)
		ios_om=ios_om|std::ios::ate;
	return ios_om;
}

template<open_mode om>
struct fstream_open_mode
{
	inline static constexpr auto value=calculate_fstream_file_open_mode(om);
};

}


template<typename fstream_type>
requires std::same_as<typename fstream_type::char_type,typename fstream_type::traits_type::char_type>
class basic_stream_file
{
public:
	using char_type = typename fstream_type::char_type;
	using traits_type = typename fstream_type::traits_type;
	using filebuf_type = __gnu_cxx::stdio_filebuf<char_type,traits_type>;
	using c_io_handle_type = basic_c_io_handle_unlocked<char_type>;
	using c_file_type = basic_c_file<c_io_handle_type>;
private:
	c_file_type bcf;
	filebuf_type hd;
	fstream_type stm;
public:
	using native_handle_type = fstream_type;
	template<open_mode om>
	basic_stream_file(c_io_handle_type&& ciohd,open_interface_t<om>):
		bcf(std::move(static_cast<c_file_type&&>(ciohd))),
		hd(bcf.native_handle(),details::fstream_open_mode<om>::value),
		stm(std::addressof(hd))
	{
		if(!stm)
			throw std::system_error(std::make_error_code(std::errc::io_error));
	}

	basic_stream_file(c_io_handle_type&& ciohd,open_mode om):
		bcf(std::move(static_cast<c_file_type&&>(ciohd))),hd(bcf.native_handle(),details::calculate_fstream_file_open_mode(om)),stm(std::addressof(hd))
	{
		if(!stm)
			throw std::system_error(std::make_error_code(std::errc::io_error));
	}
	basic_stream_file(c_io_handle_type&& ciohd,std::string_view om):
		bcf(std::move(static_cast<c_file_type&&>(ciohd))),hd(bcf.native_handle(),from_c_mode(om)),stm(std::addressof(hd))
	{
		if(!stm)
			throw std::system_error(std::make_error_code(std::errc::io_error));
	}

	template<open_mode om>
	basic_stream_file(basic_posix_io_handle<char_type>&& ciohd,open_interface_t<om>):
		basic_stream_file(c_file_type(std::move(ciohd),open_interface<om>),open_interface<om>){}

	basic_stream_file(basic_posix_io_handle<char_type>&& ciohd,open_mode om):
		basic_stream_file(c_file_type(std::move(ciohd),om),om){}

	basic_stream_file(basic_posix_io_handle<char_type>&& ciohd,std::string_view om):
		basic_stream_file(c_file_type(std::move(ciohd),om),om){}

#if defined(__WINNT__) || defined(_MSC_VER)
	template<open_mode om>
	basic_stream_file(basic_win32_io_handle<char_type>&& ciohd,open_interface_t<om>):
		basic_stream_file(basic_posix_io_handle<char_type>(std::move(ciohd),open_interface<om>),open_interface<om>){}

	basic_stream_file(basic_win32_io_handle<char_type>&& ciohd,open_mode om):
		basic_stream_file(basic_posix_io_handle<char_type>(std::move(ciohd),om),om){}

	basic_stream_file(basic_win32_io_handle<char_type>&& ciohd,std::string_view om):
		basic_stream_file(basic_posix_io_handle<char_type>(std::move(ciohd),om),om){}
#endif

	template<open_mode om,typename... Args>
	basic_stream_file(std::string_view file,open_interface_t<om>,Args&& ...args):
		basic_stream_file(c_file_type(file,open_interface<om>,std::forward<Args>(args)...),
			open_interface<om>)
	{}

	template<typename... Args>
	basic_stream_file(std::string_view file,open_mode om,Args&& ...args):
		basic_stream_file(c_file_type(file,om,std::forward<Args>(args)...),om)
	{}
	template<typename... Args>
	basic_stream_file(std::string_view file,std::string_view mode,Args&& ...args):
		basic_stream_file(c_file_type(file,mode,std::forward<Args>(args)...),mode)
	{}

	native_handle_type& native_handle()
	{
		return stm;
	}
	c_file_type& c_file()
	{
		return bcf;
	}
	filebuf_type& filebuf()
	{
		return hd;
	}

	explicit operator basic_c_io_handle_unlocked<char_type>() const
	{
		return bcf.native_handle();
	}
	explicit operator basic_posix_io_handle<char_type>() const
	{

		auto fd(
#if defined(__WINNT__) || defined(_MSC_VER)
	_fileno(bcf.native_handle())
#else
	::fileno_unlocked(bcf.native_handle())
#endif
);
		if(fd<0)
#ifdef __cpp_exceptions
			throw std::system_error(errno,std::system_category());
#else
			fast_terminate();
#endif
		return static_cast<basic_posix_io_handle<char_type>>(fd);
	}
#if defined(__WINNT__) || defined(_MSC_VER)
	explicit operator basic_win32_io_handle<char_type>() const
	{
		auto fd(
#if defined(__WINNT__) || defined(_MSC_VER)
	_fileno(bcf.native_handle())
#else
	::fileno_unlocked(bcf.native_handle())
#endif
);
		if(fd<0)
#ifdef __cpp_exceptions
			throw std::system_error(errno,std::system_category());
#else
			fast_terminate();
#endif
		auto os_handle(_get_osfhandle(fd));
		if(os_handle==-1)
#ifdef __cpp_exceptions
			throw std::system_error(errno,std::system_category());
#else
			fast_terminate();
#endif
		return static_cast<basic_win32_io_handle<char_type>>(os_handle);
	}
#endif
};

template<typename fstream_type,std::contiguous_iterator Iter>
inline Iter read(basic_stream_file<fstream_type>& cfhd,Iter begin,Iter end)
{
	return read(cfhd.c_file(),begin,end);
}

template<typename fstream_type,std::contiguous_iterator Iter>
inline auto write(basic_stream_file<fstream_type>& cfhd,Iter begin,Iter end)
{
	return write(cfhd.c_file(),begin,end);
}

template<typename fstream_type>
inline void flush(basic_stream_file<fstream_type>& cfhd)
{
	return flush(cfhd.c_file());
}
template<typename fstream_type>
inline auto oreserve(basic_stream_file<fstream_type>& cfhd,std::size_t n)
{
	using c_file_type = basic_stream_file<fstream_type>::c_file_type;
	if constexpr(fast_io::buffer_output_stream<c_file_type>)
		return oreserve(cfhd.c_file(),n);
	else
	{
		streambuf_view buf(std::addressof(cfhd.filebuf()));
		return oreserve(buf,n);
	}
}

template<typename fstream_type>
inline void orelease(basic_stream_file<fstream_type>& cfhd,std::size_t n)
{
	using c_file_type = basic_stream_file<fstream_type>::c_file_type;
	if constexpr(fast_io::buffer_output_stream<c_file_type>)
		orelease(cfhd.c_file(),n);
	else
	{
		streambuf_view buf(std::addressof(cfhd.filebuf()));
		orelease(buf,n);
	}
}

template<typename fstream_type>
inline void put(basic_stream_file<fstream_type>& cfhd,typename basic_stream_file<fstream_type>::char_type ch)
{
	using c_file_type = basic_stream_file<fstream_type>::c_file_type;
	if constexpr(fast_io::character_output_stream<c_file_type>)
		put(cfhd.c_file(),ch);
	else
	{
		streambuf_view buf(std::addressof(cfhd.filebuf()));
		put(buf,ch);
	}
}

template<typename fstream_type,typename... Args>
inline auto seek(basic_stream_file<fstream_type>& cfhd,Args&& ...args)
{
	return seek(cfhd.c_file(),std::forward<Args>(args)...);
}

template<typename fstream_type>
requires zero_copy_input_stream<typename basic_stream_file<fstream_type>::c_file_type>
inline auto zero_copy_in_handle(basic_stream_file<fstream_type>& h)
{
	return zero_copy_in_handle(h.c_file());
}

template<typename fstream_type>
requires zero_copy_output_stream<typename basic_stream_file<fstream_type>::c_file_type>
inline auto zero_copy_out_handle(basic_stream_file<fstream_type>& h)
{
	return zero_copy_out_handle(h.c_file());
}

using stream_file = basic_stream_file<std::iostream>;
using istream_file = input_file_wrapper<basic_stream_file<std::istream>>;
using ostream_file = output_file_wrapper<basic_stream_file<std::ostream>>;
using iostream_file = io_file_wrapper<stream_file>;

}