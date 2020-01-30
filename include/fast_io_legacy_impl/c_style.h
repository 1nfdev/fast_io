#pragma once
#include<cstdio>

namespace fast_io
{


template<std::integral ch_type>
requires (std::same_as<ch_type,char>||std::same_as<ch_type,wchar_t>)
class basic_c_io_handle_unlocked
{
	std::FILE *fp;
public:
	basic_c_io_handle_unlocked(std::FILE* fpp):fp(fpp){}

	using char_type = ch_type;
	using native_handle_type = std::FILE*;
	native_handle_type& native_handle()
	{
		return fp;
	}
	explicit operator basic_posix_io_handle<char_type>() const
	{
		return static_cast<basic_posix_io_handle<char_type>>(
#if defined(__WINNT__) || defined(_MSC_VER)
	_fileno(fp)
#else
	::fileno_unlocked(fp)
#endif
);
	}
#if defined(__WINNT__) || defined(_MSC_VER)
	explicit operator basic_win32_io_handle<char_type>() const
	{
		return static_cast<basic_win32_io_handle<char_type>>(_get_osfhandle(_fileno(fp)));
	}
#endif
};

using c_io_handle_unlocked = basic_c_io_handle_unlocked<char>;

/*
#ifdef _MSC_VER

struct _iobuf
{
char *_ptr;
int _cnt;
char *_base;
int _flag;
int _file;
int _charbuf;
int _bufsiz;
char *_tmpfname;
};
#endif
*/
template<std::contiguous_iterator Iter>
inline Iter read(c_io_handle_unlocked& cfhd,Iter begin,Iter end)
{
	std::size_t const count(end-begin);
	std::size_t const r(
//mingw
#ifdef _MSC_VER
	_fread_nolock
#elif _POSIX_SOURCE
	fread_unlocked
#else
	fread
#endif
	(std::to_address(begin),sizeof(*begin),count,cfhd.native_handle()));
	if(r==count||std::feof(cfhd.native_handle()))
		return begin+r;
	throw std::system_error(errno,std::generic_category());
}

template<std::contiguous_iterator Iter>
inline void write(c_io_handle_unlocked& cfhd,Iter begin,Iter end)
{
	std::size_t const count(end-begin);
	if(
#ifdef _MSC_VER
	_fwrite_nolock
#elif defined(_POSIX_SOURCE)
	fwrite_unlocked
#else
	fwrite
#endif
	(std::to_address(begin),sizeof(*begin),count,cfhd.native_handle())<count)
#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
}

template<bool err=false>
inline auto get(c_io_handle_unlocked& cfhd)
{
	auto ch(
#ifdef _MSC_VER
	_fgetc_nolock
#elif defined(_POSIX_SOURCE)
	fgetc_unlocked
#else
	fgetc
#endif
	(cfhd.native_handle()));
	if(ch==EOF)
	{
		if(
#ifdef _POSIX_SOURCE
	feof_unlocked
#else
	feof
#endif
(cfhd.native_handle()))
		{
			if constexpr(err)
				return std::pair<typename c_io_handle_unlocked::char_type,bool>{0,true};
			else
				throw eof();
		}
		throw std::system_error(errno,std::system_category());
	}
	if constexpr(err)
		return std::pair<typename c_io_handle_unlocked::char_type,bool>
		{
			std::char_traits<typename c_io_handle_unlocked::char_type>::to_char_type(ch),false
		};
	else
		return std::char_traits<typename c_io_handle_unlocked::char_type>::to_char_type(ch);
}

inline void put(c_io_handle_unlocked& cfhd,typename c_io_handle_unlocked::char_type ch)
{
	if(
#if defined(_MSC_VER)
		_fputc_nolock
#elif defined(_POSIX_SOURCE)
		fputc_unlocked
#else
		fputc
#endif
	(static_cast<char>(ch),cfhd.native_handle())==EOF)

#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
}

inline void flush(c_io_handle_unlocked& cfhd)
{
	if(
#if defined(__WINNT__) || defined(_MSC_VER)
		_fflush_nolock
#elif defined(_POSIX_SOURCE)
		fflush_unlocked
#else
		fflush
#endif
	(cfhd.native_handle()))
#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
}

template<typename T,std::integral U>
inline void seek(c_io_handle_unlocked& cfhd,seek_type_t<T>,U i,seekdir s=seekdir::beg)
{
	if(
#if defined(__WINNT__) || defined(_MSC_VER)
		_fseek_nolock
#elif defined(_POSIX_SOURCE)
		fseek_unlocked
#else
		fseek
#endif
	(cfhd.native_handle(),seek_precondition<long,T,char>(i),static_cast<int>(s)))
		throw std::system_error(errno,std::system_category()); 
}

template<std::integral U>
inline void seek(c_io_handle_unlocked& cfhd,U i,seekdir s=seekdir::beg)
{
	seek(cfhd,seek_type<char>,i,s);
}


//Exploiting the library internal implementation for performance since the stdio.h performance is SO TERRIBLE
#ifdef __MINGW32__
//cannot convert to char8_t due to strict aliasing rule violation
inline char* oreserve(c_io_handle_unlocked& cfhd,std::size_t n)
{
	auto& h(*cfhd.native_handle());
	if((h._cnt-=n)<=0) [[unlikely]]
	{
		h._cnt+=n;
		return nullptr;
	}
	return h._ptr+=n;
}

inline void orelease(c_io_handle_unlocked& cfhd,std::size_t n)
{
	auto& h(*(_iobuf*)cfhd.native_handle());
	h._cnt+=n;
	h._ptr-=n;
}
#elif defined(__GNU_LIBRARY__)
inline char* oreserve(c_io_handle_unlocked& cfhd,std::size_t n)
{
	auto& h(*cfhd.native_handle());
	if(h._IO_write_end<=h._IO_write_ptr+n)
		return nullptr;
	return h._IO_write_ptr+=n;
}

inline void orelease(c_io_handle_unlocked& cfhd,std::size_t n)
{
	cfhd.native_handle()->_IO_write_ptr-=n;
}
#endif

class c_io_lock_guard;

template<std::integral ch_type>
class basic_c_io_handle
{
	std::FILE *fp;
public:
	using lock_guard_type = c_io_lock_guard;
	basic_c_io_handle(std::FILE* fpp):fp(fpp){}
	using char_type = ch_type;
	using native_handle_type = std::FILE*;
	native_handle_type& native_handle()
	{
		return fp;
	}
	explicit operator basic_posix_io_handle<char_type>() const
	{
		return static_cast<basic_posix_io_handle<char_type>>(
#if defined(__WINNT__) || defined(_MSC_VER)
	_fileno(fp)
#else
	::fileno(fp)
#endif
);
	}
#if defined(__WINNT__) || defined(_MSC_VER)
	explicit operator basic_win32_io_handle<char_type>() const
	{
		return static_cast<basic_win32_io_handle<char_type>>(_get_osfhandle(_fileno(fp)));
	}
#endif
};

using c_io_handle=basic_c_io_handle<char>;

inline auto mutex(c_io_handle& h)
{
	return h.native_handle();
}

inline auto unlocked_handle(c_io_handle& h)
{
	return c_io_handle_unlocked(h.native_handle());
}

class c_io_lock_guard
{
	std::FILE* const fp;
public:
	c_io_lock_guard(std::FILE* f):fp(f)
	{
#if defined(__WINNT__) || defined(_MSC_VER)
		_lock_file(fp);
#else
		flockfile(fp);
#endif
	}
	c_io_lock_guard(c_io_lock_guard const&) = delete;
	c_io_lock_guard& operator=(c_io_lock_guard const&) = delete;
	~c_io_lock_guard()
	{
#if defined(__WINNT__) || defined(_MSC_VER)
		_unlock_file(fp);
#else
		funlockfile(fp);
#endif
	}
};


template<std::contiguous_iterator Iter>
inline Iter read(c_io_handle& cfhd,Iter begin,Iter end)
{
	std::size_t const count(end-begin);
	std::size_t const r(std::fread(std::to_address(begin),sizeof(*begin),count,cfhd.native_handle()));
	if(r==count||std::feof(cfhd.native_handle()))
		return begin+r;
#ifdef __cpp_exceptions
	throw std::system_error(errno,std::system_category());
#else
	fast_terminate();
#endif
}

template<std::contiguous_iterator Iter>
inline void write(c_io_handle& cfhd,Iter begin,Iter end)
{
	std::size_t const count(end-begin);
	if(std::fwrite(std::to_address(begin),sizeof(*begin),count,cfhd.native_handle())<count)
#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
}


inline void put(c_io_handle& cfhd,typename c_io_handle::char_type ch)
{
	if(std::fputc(ch,cfhd.native_handle())==EOF)
#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
}

inline void flush(c_io_handle& cfhd)
{
	if(std::fflush(cfhd.native_handle()))
#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
}

template<typename T,std::integral U>
inline void seek(c_io_handle& cfhd,seek_type_t<T>,U i,seekdir s=seekdir::beg)
{
	if(std::fseek(cfhd.native_handle(),seek_precondition<long,T,typename c_io_handle::char_type>(i),static_cast<int>(s)))
#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
}

template<std::integral U>
inline void seek(c_io_handle& cfhd,U i,seekdir s=seekdir::beg)
{
	seek(cfhd,seek_type<typename c_io_handle::char_type>,i,s);
}

template<typename T>
class basic_c_file:public T
{
	void close_impl() noexcept
	{
		if(native_handle())
			std::fclose(native_handle());
	}
public:
	using T::native_handle;
	using T::char_type;
	using T::native_handle_type;
	template<typename ...Args>
	basic_c_file(native_interface_t,Args&& ...args):T(std::fopen(std::forward<Args>(args)...))
	{
		if(native_handle()==nullptr)
#ifdef __cpp_exceptions
			throw std::system_error(errno,std::system_category());
#else
			fast_terminate();
#endif
	}


	basic_c_file(std::string_view name,std::string_view mode):T(std::fopen(name.data(),mode.data()))
	{
		if(native_handle()==nullptr)
#ifdef __cpp_exceptions
			throw std::system_error(errno,std::system_category());
#else
			fast_terminate();
#endif
	}
	basic_c_file(std::string_view file,open::mode const& m):basic_c_file(file,c_style(m))
	{
		if(with_ate(m))
			seek(*this,0,seekdir::end);
	}
	template<std::size_t om>
	basic_c_file(std::string_view name,open::interface_t<om>):basic_c_file(name,open::c_style_interface_t<om>::mode)
	{
		if constexpr (with_ate(open::mode(om)))
			seek(*this,0,seekdir::end);
	}

//fdopen interface

	basic_c_file(basic_posix_io_handle<typename T::char_type>&& posix_handle,std::string_view mode):T(
#if defined(__WINNT__) || defined(_MSC_VER)
			::_fdopen
#else
			::fdopen
#endif
(posix_handle.native_handle(),mode.data()))
	{
		if(native_handle()==nullptr)
#ifdef __cpp_exceptions
			throw std::system_error(errno,std::system_category());
#else
			fast_terminate();
#endif
		posix_handle.reset();
	}

	basic_c_file(basic_posix_io_handle<typename T::char_type>&& posix_handle,open::mode const& m):basic_c_file(std::move(posix_handle),c_style(m)){}
	template<std::size_t om>
	basic_c_file(basic_posix_io_handle<typename T::char_type>&& posix_handle,open::interface_t<om>):basic_c_file(std::move(posix_handle),open::c_style_interface_t<om>::mode){}

#if defined(__WINNT__) || defined(_MSC_VER)
//windows specific. open posix file from win32 io handle
	basic_c_file(basic_win32_io_handle<typename T::char_type>&& win32_handle,std::string_view mode):
		basic_c_file(basic_posix_file<typename T::char_type>(std::move(win32_handle),mode),mode)
	{
	}
	basic_c_file(basic_win32_io_handle<typename T::char_type>&& win32_handle,open::mode const& m):
		basic_c_file(basic_posix_file<typename T::char_type>(std::move(win32_handle),m),m)
	{
	}
	template<std::size_t om>
	basic_c_file(basic_win32_io_handle<typename T::char_type>&& win32_handle,open::interface_t<om>):
		basic_c_file(basic_posix_file<typename T::char_type>(std::move(win32_handle),open::interface<om>),open::c_style_interface_t<om>::mode)
	{
	}
#endif
	basic_c_file(basic_c_file const&)=delete;
	basic_c_file& operator=(basic_c_file const&)=delete;
	basic_c_file(basic_c_file&& b) noexcept : T(b.native_handle())
	{
		b.native_handle() = nullptr;
	}
	basic_c_file& operator=(basic_c_file&& b) noexcept
	{
		if(std::addressof(b)!=this)
		{
			close_impl();
			native_handle()=b.native_handle();
			b.native_handle() = nullptr;
		}
		return *this;
	}
	~basic_c_file()
	{
		close_impl();
	}
};

template<typename... Args>
requires requires(std::FILE* fp,Args&& ...args)
{
	std::fprintf(fp,std::forward<Args>(args)...);
}
inline auto fprintf(c_io_handle& h,Args&& ...args)
{
	auto v(std::fprintf(h.native_handle(),std::forward<Args>(args)...));
// forgetting checking printf/fprintf return value is a common mistake and a huge source of security vulnerability
	if(v<0)
#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
	return v;
}


template<typename... Args>
requires requires(std::FILE* fp,Args&& ...args)
{
	std::fscanf(fp,std::forward<Args>(args)...);
}
inline auto fscanf(c_io_handle& h,Args&& ...args)
{
	auto v(std::fscanf(h.native_handle(),std::forward<Args>(args)...));
// forgetting checking scanf/fscanf return value is a common mistake and a huge source of security vulnerability
	if(v<0)
#ifdef __cpp_exceptions
		throw std::system_error(errno,std::system_category());
#else
		fast_terminate();
#endif
	return v;
}


using c_file = basic_c_file<c_io_handle>;
using c_file_unlocked = basic_c_file<c_io_handle_unlocked>;

#ifdef __linux__
template<std::integral ch_type>
inline auto zero_copy_in_handle(basic_c_io_handle_unlocked<ch_type>& h)
{
	return ::fileno_unlocked(h.native_handle());
}
template<std::integral ch_type>
inline auto zero_copy_out_handle(basic_c_io_handle_unlocked<ch_type>& h)
{
	return ::fileno_unlocked(h.native_handle());
}
#endif

}
