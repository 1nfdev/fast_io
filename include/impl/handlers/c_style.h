#pragma once
#include<cstdio>
#include<cstddef>
#include"../mode.h"
#include<system_error>
//#include"exploiting_file_ptr.h"

namespace fast_io
{

class c_style_io_handle
{
	std::FILE *fp;
protected:
	native_handle_type& protected_native_handle()
	{
		return fp;
	}
public:
	c_style_io_handle(std::FILE* fpp):fp(fpp){}
	using traits_type = std::char_traits<char>;
	using native_handle_type = std::FILE*;
	native_handle_type native_handle() const
	{
		return fp;
	}
	bool eof() const
	{
		return std::feof(fp);
	}
	template<typename ContiguousIterator>
	ContiguousIterator read(ContiguousIterator begin,ContiguousIterator end)
	{
		std::size_t const count(end-begin);
		std::size_t const r(std::fread(std::addressof(*begin),sizeof(*begin),count,fp));
		if(r==count||std::feof(fp))
			return begin+r;
		throw std::system_error(errno,std::generic_category());
	}

	template<typename ContiguousIterator>
	void write(ContiguousIterator begin,ContiguousIterator end)
	{
		std::size_t const count(end-begin);
		if(std::fwrite(std::addressof(*begin),sizeof(*begin),count,fp)<count)
			throw std::system_error(errno,std::generic_category());
	}
	traits_type::int_type get()
	{
		return hacking::filestar_fast_get(fp);
	}
	void put(traits_type::int_type ch)
	{
		if(fputc(ch,fp)==EOF)
			throw std::system_error(errno,std::system_category());
	}
	template<typename ...Args>
	void printf(Args&& ...args)
	{
		if(std::fprintf(fp,std::forward<Args>(args)...)<0)
			throw std::system_error(errno,std::system_category());		
	}
	void flush()
	{
		if(std::fflush(fp))
			throw std::system_error(errno,std::system_category());
	}
};

class c_style_file:public c_style_io_handle
{
	void close_impl() noexcept
	{
		if(native_handle())
			std::fclose(native_handle());
	}
public:
	using traits_type = std::char_traits<char>;
	using native_handle_type = std::FILE*;
	template<typename ...Args>
	c_style_file(native_interface_t,Args&& ...args):c_style_io_handle(std::fopen(std::forward<Args>(args)...))
	{
		if(native_handle()==nullptr)
			throw std::system_error(errno,std::generic_category());
	}
	c_style_file(std::string_view name,std::string_view mode):c_style_file(native_interface,name.data(),mode.data()){}
	c_style_file(std::string_view file,open::mode const& m):c_style_file(file,c_style(m)){}
	template<std::size_t om>
	c_style_file(std::string_view name,open::interface_t<om>):c_style_file(name,open::interface_t<om>::c_style){}
	c_style_file(c_style_file const&)=delete;
	c_style_file& operator=(c_style_file const&)=delete;
	c_style_file(c_style_file&& b) noexcept : c_style_io_handle(b.native_handle())
	{
		b.protected_native_handle() = nullptr;
	}
	c_style_file& operator=(c_style_file&& b) noexcept
	{
		if(&b!=this)
		{
			close_impl();
			protected_native_handle()=b.native_handle();
			b.protected_native_handle() = nullptr;
		}
		return *this;
	}
	~c_style_file()
	{
		close_impl();
	}
};
}