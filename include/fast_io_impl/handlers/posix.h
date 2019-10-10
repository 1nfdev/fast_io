#pragma once
#include<unistd.h>
#include<fcntl.h>
#ifdef __linux__
#include<sys/sendfile.h>
#endif
namespace fast_io
{
	
namespace details
{
inline constexpr int calculate_posix_open_mode(open::mode const &om)
{
	using namespace open;
	std::size_t value(remove_ate(om).value);
	int mode(0);
	if(value&binary.value)
	{
#ifdef O_BINARY
		mode |= O_BINARY;
#endif
		value &= ~binary.value;
	}
	if(value&excl.value)
	{
		mode |= O_CREAT  | O_EXCL;
		value &= ~excl.value;
	}
	if(value&trunc.value)
	{
		mode |= O_TRUNC;
		value &= ~trunc.value;
	}
	if(value&direct.value)
	{
#ifdef O_DIRECT
		mode |= O_DIRECT;
#endif
		value &= ~direct.value;
	}
	if(value&sync.value)
	{
#ifdef O_SYNC
		mode |= O_SYNC;
#endif
		value &= ~sync.value;
	}
	switch(value)
	{
//Action if file already exists;	Action if file does not exist;	c-style mode;	Explanation
//Read from start;	Failure to open;	"r";	Open a file for reading
	case in:
		return mode | O_RDONLY;
//Destroy contents;	Create new;	"w";	Create a file for writing
	case out:
		return mode | O_WRONLY | O_CREAT | O_TRUNC;
//Append to file;	Create new;	"a";	Append to a file
	case app:
	case out|app:
		return mode | O_WRONLY | O_CREAT | O_APPEND;
//Read from start;	Error;	"r+";		Open a file for read/write
	case out|in:
		return mode | O_RDWR;
//Write to end;	Create new;	"a+";	Open a file for read/write
	case out|in|app:
	case in|app:
		return mode | O_RDWR | O_CREAT | O_APPEND;
//Destroy contents;	Error;	"wx";	Create a file for writing
	default:
		throw std::runtime_error("unknown posix file openmode");
	}
}
template<std::size_t om>
struct posix_file_openmode
{
	static int constexpr mode = calculate_posix_open_mode(om);
};
}

class posix_io_handle
{
	int fd;
protected:
	void close_impl() noexcept
	{
		if(fd!=-1)
			close(fd);
	}
	auto& protected_native_handle()
	{
		return fd;
	}
public:
	using char_type = char;
	using native_handle_type = int;
	native_handle_type native_handle() const
	{
		return fd;
	}
	posix_io_handle() = default;
	posix_io_handle(int fdd):fd(fdd){}
	template<typename ContiguousIterator>
	ContiguousIterator read(ContiguousIterator begin,ContiguousIterator end)
	{
		auto read_bytes(::read(fd,std::addressof(*begin),(end-begin)*sizeof(*begin)));
		if(read_bytes==-1)
			throw std::system_error(errno,std::generic_category());
		return begin+(read_bytes/sizeof(*begin));
	}
	template<typename ContiguousIterator>
	ContiguousIterator write(ContiguousIterator begin,ContiguousIterator end)
	{
		auto write_bytes(::write(fd,std::addressof(*begin),(end-begin)*sizeof(*begin)));
		if(write_bytes==-1)
			throw std::system_error(errno,std::generic_category());
		return begin+(write_bytes/sizeof(*begin));
	}
	template<typename T,std::integral R>
	void seek(seek_type_t<T>,R i,seekdir s=seekdir::beg)
	{
		if(::lseek64(fd,seek_precondition<off64_t,T,char_type>(i),static_cast<int>(s))==-1)
			throw std::system_error(errno,std::generic_category()); 
	}
	template<std::integral R>
	void seek(R i,seekdir s=seekdir::beg)
	{
		seek(seek_type<char_type>,i,s);
	}
	void flush()
	{
		// no need fsync. OS can deal with it
//		if(::fsync(fd)==-1)
//			throw std::system_error(errno,std::generic_category());
	}

	posix_io_handle(posix_io_handle const& dp):fd(dup(dp.fd))
	{
		if(fd<0)
			throw std::system_error(errno,std::generic_category());
	}
	posix_io_handle& operator=(posix_io_handle const& dp)
	{
		auto newfd(dup2(dp.fd,fd));
		if(newfd<0)
			throw std::system_error(errno,std::generic_category());
		fd=newfd;
		return *this;
	}
	posix_io_handle(posix_io_handle&& b) noexcept : posix_io_handle(b.fd)
	{
		b.fd = -1;
	}
	posix_io_handle& operator=(posix_io_handle&& b) noexcept
	{
		if(std::addressof(b)!=this)
		{
			close_impl();
			fd=b.fd;
			b.fd = -1;
		}
		return *this;
	}
#ifdef __linux__
	auto zero_copy_in_handle()
	{
		return fd;
	}
	auto zero_copy_out_handle()
	{
		return fd;
	}
#endif
};

class posix_file:public posix_io_handle
{
public:
	using char_type = posix_io_handle::char_type;
	using native_handle_type = posix_io_handle::native_handle_type;
	template<typename ...Args>
	posix_file(native_interface_t,Args&& ...args):posix_io_handle(::open(std::forward<Args>(args)...))
	{
		if(native_handle()==-1)
			throw std::system_error(errno,std::generic_category());
	}
	template<std::size_t om>
	posix_file(std::string_view file,open::interface_t<om>):posix_file(native_interface,file.data(),details::posix_file_openmode<om>::mode,420)
	{
		if constexpr (with_ate(open::mode(om)))
			seek(0,seekdir::end);
	}
	//potential support modification prv in the future
	posix_file(std::string_view file,open::mode const& m):posix_file(native_interface,file.data(),details::calculate_posix_open_mode(m),420)
	{
		if(with_ate(m))
			seek(0,seekdir::end);
	}
	posix_file(std::string_view file,std::string_view mode):posix_file(file,fast_io::open::c_style(mode)){}
	~posix_file()
	{
		posix_io_handle::close_impl();
	}
};

class posix_pipe_unique:public posix_io_handle
{
public:
	using char_type = char;
	using native_handle_type = int;
	void close()
	{
		posix_io_handle::close_impl();
		protected_native_handle() = -1;
	}
	~posix_pipe_unique()
	{
		posix_io_handle::close_impl();
	}
};

class posix_pipe
{
public:
	using char_type = char;
	using native_handle_type = std::array<posix_pipe_unique,2>;
private:
	native_handle_type pipes;
public:
	posix_pipe()
	{
#ifdef _WIN32_WINNT
		if(_pipe(static_cast<int*>(static_cast<void*>(pipes.data())),1048576,_O_BINARY)==-1)
#else
		if(::pipe(static_cast<int*>(static_cast<void*>(pipes.data())))==-1)
#endif
			throw std::system_error(errno,std::generic_category());
	}
	template<std::size_t om>
	posix_pipe(open::interface_t<om>):posix_pipe()
	{
		auto constexpr omb(om&~open::binary.value);
		static_assert(omb==open::in.value||omb==open::out.value||omb==(open::in.value|open::out.value),"pipe open mode must be in or out");
		if constexpr (!(om&~open::in.value)&&(om&~open::out.value))
			pipes.front().close();
		if constexpr ((om&~open::in.value)&&!(om&~open::out.value))
			pipes.back().close();
	}
	auto& native_handle()
	{
		return pipes;
	}
	void flush()
	{
	}
	auto& in()
	{
		return pipes.front();
	}
	auto& out()
	{
		return pipes.back();
	}
	auto zero_copy_in_handle()
	{
		return in().native_handle();
	}
	auto zero_copy_out_handle()
	{
		return out().native_handle();
	}
	template<typename ContiguousIterator>
	ContiguousIterator read(ContiguousIterator begin,ContiguousIterator end)
	{
		return pipes.front().read(begin,end);
	}
	template<typename ContiguousIterator>
	void write(ContiguousIterator begin,ContiguousIterator end)
	{
		pipes.back().write(begin,end);
	}
};

#ifndef __WINNT__
using system_file = posix_file;
using system_io_handle = posix_io_handle;
using system_pipe_unique = posix_pipe_unique;
using system_pipe = posix_pipe;
inline int constexpr native_stdin_number = 0;
inline int constexpr native_stdout_number = 1;
inline int constexpr native_stderr_number = 2;
#endif
#ifdef __linux__

//zero copy IO for linux
namespace details
{
template<zero_copy_output_stream output,zero_copy_input_stream input>
inline std::uint_least64_t zero_copy_transmit_once(output& outp,input& inp,std::uint_least64_t bytes)
{
	auto transmitted_bytes(::sendfile64(outp.zero_copy_out_handle(),inp.zero_copy_in_handle(),nullptr,bytes));
	if(transmitted_bytes==-1)
		throw std::system_error(errno,std::generic_category());
	return transmitted_bytes;
}
}


template<zero_copy_output_stream output,zero_copy_input_stream input>
inline std::uint_least64_t zero_copy_transmit(output& outp,input& inp,std::uint_least64_t bytes)
{
	std::uint_least64_t constexpr maximum_transmit_bytes(std::numeric_limits<std::int64_t>::max());
	std::uint_least64_t transmitted(0);
	for(;bytes;)
	{
		std::uint_least64_t should_transfer(maximum_transmit_bytes);
		if(bytes<should_transfer)
			should_transfer=bytes;
		std::uint_least64_t transferred_this_round(details::zero_copy_transmit_once(outp,inp,should_transfer));
		transmitted+=transferred_this_round;
		if(transferred_this_round!=should_transfer)
			return transmitted;
		bytes-=transferred_this_round;
	}
	return transmitted;
	
}
template<zero_copy_output_stream output,zero_copy_input_stream input>
inline std::uint_least64_t zero_copy_transmit(output& outp,input& inp)
{
	std::uint_least64_t constexpr maximum_transmit_bytes(std::numeric_limits<std::int64_t>::max());
	for(std::uint_least64_t transmitted(0);;)
	{
		std::uint_least64_t transferred_this_round(details::zero_copy_transmit_once(outp,inp,maximum_transmit_bytes));
		transmitted+=transferred_this_round;
		if(transferred_this_round!=maximum_transmit_bytes)
			return transmitted;
	}
}
#endif

}
