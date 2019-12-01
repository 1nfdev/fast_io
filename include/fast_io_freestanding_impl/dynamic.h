#pragma once

namespace fast_io
{

template<std::integral T>
class basic_dynamic_stream
{
public:
	using char_type = T;
private:
	struct base
	{
		virtual char_type* reads_impl(char_type*,char_type*) = 0;
		virtual void writes_impl(char_type const*,char_type const*) = 0;
		virtual void flush_impl() = 0;
		virtual base* clone() = 0;
		virtual ~base() = default;
	};
	template<stream stm>
	struct derv:base
	{
		stm io;
		template<typename ...Args>
		derv(std::in_place_type_t<stm>,Args&& ...args):io(std::forward<Args>(args)...){}
		char_type* reads_impl(char_type* b,char_type* e)
		{
			if constexpr(input_stream<stm>)
				return reads(io,b,e);
			else
				throw std::system_error(EPERM,std::generic_category());
		}
		void writes_impl(char_type const* b,char_type const* e)
		{
			if constexpr(output_stream<stm>)
				writes(io,b,e);
			else
				throw std::system_error(EPERM,std::generic_category());
		}
		void flush_impl()
		{
			if constexpr(output_stream<stm>)
				flush(io);
			else
				throw std::system_error(EPERM,std::generic_category());
		}
		base* clone()
		{
			if constexpr(std::copyable<stm>)
				return new derv<stm>(std::in_place_type<stm>,io);
			else
				throw std::system_error(EPERM,std::generic_category());
		}
	};
	base* ptr=nullptr;
public:
	using opaque_base = base;
	basic_dynamic_stream()=default;
	basic_dynamic_stream(void* p):ptr(reinterpret_cast<base*>(ptr)){}
	auto release() noexcept
	{
		auto temp(ptr);
		ptr=nullptr;
		return temp;
	}
	auto opaque_base_pointer()
	{
		return ptr;
	}
	auto opaque_base_pointer() const
	{
		return ptr;
	}
	basic_dynamic_stream(basic_dynamic_stream const& b):basic_dynamic_stream(b.ptr->clone()){}
	basic_dynamic_stream& operator=(basic_dynamic_stream const& b)
	{
		auto newp(b.ptr->clone());
		delete ptr;
		ptr=newp;
		return *this;
	}
	basic_dynamic_stream(basic_dynamic_stream&& b) noexcept:ptr(b.ptr)
	{
		b.ptr=nullptr;
	}
	basic_dynamic_stream& operator=(basic_dynamic_stream&& b) noexcept
	{
		if(b.ptr!=ptr)
		{
			delete ptr;
			ptr=b.ptr;
			b.ptr=nullptr;
		}
		return *this;
	}
	~basic_dynamic_stream()
	{
		delete ptr;
	}
	template<stream P>
	requires (!std::same_as<basic_dynamic_stream,P>)
	basic_dynamic_stream(P p):ptr(new derv<P>(std::in_place_type<P>,std::move(p))){}
	template<stream P,typename ...Args>
	basic_dynamic_stream(std::in_place_type_t<P>,Args&& ...args):
		ptr(new derv<P>(std::in_place_type<P>,std::forward<Args>(args)...)){}
};


template<std::integral char_type,std::contiguous_iterator Iter>
inline Iter reads(basic_dynamic_stream<char_type>& io,Iter b,Iter e)
{
	char_type *pb(static_cast<char_type*>(static_cast<void*>(std::to_address(b))));
	char_type *pe(static_cast<char_type*>(static_cast<void*>(std::to_address(e))));
	return b+(io.opaque_base_pointer()->reads_impl(pb,pe)-pb)*sizeof(*b)/sizeof(char_type);
}

template<std::integral char_type,std::contiguous_iterator Iter>
inline void writes(basic_dynamic_stream<char_type>& io,Iter b,Iter e)
{
	io.opaque_base_pointer()->writes_impl(static_cast<char_type const*>(static_cast<void const*>(std::to_address(b))),
			static_cast<char_type const*>(static_cast<void const*>(std::to_address(e))));
}

template<std::integral char_type>
inline void flush(basic_dynamic_stream<char_type>& io) { return io.opaque_base_pointer()->flush_impl();}

using dynamic_stream = basic_dynamic_stream<char>;

}