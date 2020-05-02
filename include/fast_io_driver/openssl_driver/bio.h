#pragma once
struct bio_method_st {
    int type;
    char const*name;
    int (*bwrite) (BIO *, const char *, size_t, size_t *);
    int (*bwrite_old) (BIO *, const char *, int);
    int (*bread) (BIO *, char *, size_t, size_t *);
    int (*bread_old) (BIO *, char *, int);
    int (*bputs) (BIO *, const char *);
    int (*bgets) (BIO *, char *, int);
    long (*ctrl) (BIO *, int, long, void *);
    int (*create) (BIO *);
    int (*destroy) (BIO *);
    long (*callback_ctrl) (BIO *, int, BIO_info_cb *);
};

namespace fast_io::openssl
{

template<stream stm>
struct fast_io_bio_method_t
{
	bio_method_st method{};
	explicit fast_io_bio_method_t()
	{
		if constexpr(input_stream<stm>)
		{
			method.bread=[](BIO* bbio,char* buf,std::size_t size,std::size_t* readd) noexcept->int
			{
				try
				{
					*readd=(read(*bit_cast<stm*>(BIO_get_data(bbio)),buf,buf+size)-buf)*sizeof(typename stm::char_type);
					return 0;
				}
				catch(...)
				{
					return -1;
				}
			};
		}
		if constexpr(output_stream<stm>)
		{
			method.bwrite=[](BIO* bbio,char const* buf,std::size_t size,std::size_t* written) noexcept->int
			{
				try
				{
					*written=(write(*bit_cast<stm*>(BIO_get_data(bbio)),buf,buf+size)-buf)*sizeof(typename stm::char_type);
					return 0;
				}
				catch(...)
				{
					return -1;
				}
			};
		}
		method.destroy=[](BIO* bbio) noexcept -> int
		{
			delete bit_cast<stm*>(BIO_get_data(bbio));
			return 0;
		};
		method.name=typeid(stm).name();
		constexpr int value(BIO_TYPE_DESCRIPTOR-BIO_TYPE_START);
		static_assert(0<value);
		method.type=static_cast<int>(typeid(stm).hash_code()%value+BIO_TYPE_START);
	}
};

template<stream stm>
fast_io_bio_method_t<stm> const fast_io_bio_method{};

template<std::integral ch_type>
class basic_bio_io_observer
{
public:
	using native_handle_type = BIO*;
	using char_type = ch_type;
	native_handle_type bio{};
	constexpr operator bool() const noexcept
	{
		return bio;
	}
	constexpr auto& native_handle() const noexcept
	{
		return bio;
	}
	constexpr auto& native_handle() noexcept
	{
		return bio;
	}
	constexpr auto release() noexcept
	{
		auto temp{bio};
		bio=nullptr;
		return temp;
	}
};

template<std::integral ch_type>
class basic_bio_file:public basic_bio_io_observer<ch_type>
{
public:
	using native_handle_type = BIO*;
	using char_type = ch_type;
	constexpr basic_bio_file()=default;
	constexpr basic_bio_file(native_handle_type bio):basic_bio_io_observer<ch_type>(bio){}
	template<stream stm,typename ...Args>
	requires std::constructible_from<stm,Args...>
	basic_bio_file(c_file_cookie_t,std::in_place_type_t<stm>,Args&& ...args):basic_bio_io_observer<ch_type>(BIO_new(std::addressof(fast_io_bio_method<stm>.method)))
	{
//		::debug_println(__FILE__," ",__LINE__);
		if(this->native_handle()==nullptr)[[unlikely]]
			throw openssl_error("BIO_new() failed");
		basic_bio_file<ch_type> self(this->native_handle());
		BIO_set_data(this->native_handle(),bit_cast<void*>(new stm(std::forward<Args>(args)...)));
//		::debug_println(__FILE__," ",__LINE__);

		self.release();
	}
	basic_bio_file(basic_bio_file const&)=delete;
	basic_bio_file& operator=(basic_bio_file const&)=delete;
	constexpr basic_bio_file(basic_bio_file&& bf) noexcept:basic_bio_io_observer<ch_type>(bf.native_handle())
	{
		bf.native_handle()=nullptr;
	}
	basic_bio_file& operator=(basic_bio_file&& bf) noexcept
	{
		if(bf.native_handle()==this->native_handle())
			return *this;
		if(this->native_handle())[[likely]]
			BIO_free(this->native_handle());
		this->native_handle()=bf.native_handle();
		bf.native_handle()=nullptr;
		return *this;
	}
	~basic_bio_file()
	{
		if(this->native_handle())[[likely]]
			BIO_free(this->native_handle());
	}
};

using bio_io_observer =  basic_bio_io_observer<char>;
using bio_file =  basic_bio_file<char>;

template<std::integral ch_type,std::contiguous_iterator Iter>
inline Iter read(basic_bio_io_observer<ch_type> iob,Iter begin,Iter end)
{
	std::size_t read_bytes{};
	if(BIO_write_ex(iob.native_handle(),std::to_address(begin),
		sizeof(*begin)*(std::to_address(end)-std::to_address(begin)),std::addressof(read_bytes))==-1)
		throw openssl_error("BIO_read_ex failed()");
	return begin+read_bytes/sizeof(*begin);
}

template<std::integral ch_type,std::contiguous_iterator Iter>
inline Iter write(basic_bio_io_observer<ch_type> iob,Iter begin,Iter end)
{
	std::size_t written_bytes{};
	if(BIO_write_ex(iob.native_handle(),std::to_address(begin),
		sizeof(*begin)*(std::to_address(end)-std::to_address(begin)),std::addressof(written_bytes))==-1)
		throw openssl_error("BIO_write_ex failed()");
	return begin+written_bytes/sizeof(*begin);
}

static_assert(input_stream<bio_file>);
static_assert(output_stream<bio_file>);

template<output_stream output,std::integral ch_type>
constexpr void print_define(output& out,basic_bio_io_observer<ch_type> bio)
{
	print(out,fast_io::unsigned_view(bio.native_handle()));
}

}