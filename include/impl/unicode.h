#pragma once
#include"concept.h"
#include<bitset>
#include<stdexcept>
#include<array>
namespace fast_io
{

template<typename T,typename CharT,typename Traits = std::char_traits<CharT>>
requires input_stream<T>()||output_stream<T>()
class unicode_view
{
	T& ib;
public:
	using native_interface_t = T;
	using traits_type = Traits;
	using native_traits_type = typename native_interface_t::traits_type;
private:
	using char_type = typename traits_type::char_type;
	using int_type = typename traits_type::int_type;

	using native_char_type = typename native_traits_type::char_type;
	using native_int_type = typename native_traits_type::int_type;
public:
	constexpr unicode_view(T& ibv):ib(ibv){}
	constexpr auto& native_handle()
	{
		return ib;
	}
	constexpr auto eof() const requires standard_input_stream<T>()
	{
		return ib.eof();
	}
	constexpr int_type get() requires standard_input_stream<T>()
	{
		auto ch(ib.get());
		if(ch==native_traits_type::eof())
			return traits_type::eof();
		auto constexpr ch_bits(sizeof(native_char_type)*8);
		using fake_char_t = std::make_unsigned_t<char_type>;
		union
		{
			fake_char_t fake_char;
			std::bitset<ch_bits> bts;
		}u{static_cast<fake_char_t>(ch)};
		if(!u.bts.test(ch_bits-1))
			return u.fake_char;
		auto constexpr ch_bits_m2(ch_bits-2);
		auto constexpr limitm1((static_cast<std::size_t>(1)<<ch_bits_m2)-1);
		if(!u.bts.test(ch_bits_m2))
			throw std::runtime_error("not a utf8 character");
		u.bts.reset(ch_bits-1);
		std::size_t pos(ch_bits_m2-1);
		for(;pos<ch_bits&&u.bts.test(pos);--pos)
			u.bts.reset(pos);
		std::size_t bytes(ch_bits_m2-pos);
		char_type converted_ch(u.fake_char);
		for(std::size_t i(0);i!=bytes;++i)
		{
			auto c(ib.get());
			if(c==native_traits_type::eof())
				throw std::runtime_error("end of file before complete reading a utf8 character");
			fake_char_t t{static_cast<fake_char_t>(c)};
			if((t>>ch_bits_m2)==2)
				converted_ch=(converted_ch<<ch_bits_m2)|(t&limitm1);
			else
				throw std::runtime_error("not a utf8 character");
		}
		return traits_type::to_int_type(converted_ch);
	}
	template<typename Contiguous_iterator>
	constexpr Contiguous_iterator read(Contiguous_iterator b,Contiguous_iterator e) requires standard_input_stream<T>()
	{
		auto pb(static_cast<char_type*>(static_cast<void*>(std::addressof(*b))));
		auto pe(pb+(e-b)*sizeof(*b)/sizeof(char_type));
		auto pi(pb);
		for(;pi!=pe;++pi)
			*pb=traits_type::to_char_type(get());
		return b+(pi-pb)*sizeof(char_type)/sizeof(*b);
	}
	constexpr void put(int_type ch) requires standard_output_stream<T>()
	{
		auto constexpr bytes(sizeof(typename native_traits_type::char_type)*8);
		auto constexpr lshift1(bytes-1);
		auto constexpr limit1(static_cast<std::size_t>(1)<<lshift1);
		if(ch<limit1)
		{
			ib.put(ch);
			return;
		}
		auto constexpr limitm1(limit1-1);
		auto constexpr lshift2(bytes-2);
		auto constexpr limit2(static_cast<std::size_t>(1)<<lshift2);
		auto constexpr limitm2(limit2-1);

/*		auto constexpr lshift(bytes-2);
		auto constexpr limitm1((static_cast<std::size_t>(1)<<lshift)-1);
		for(;limitm1<ch;ch>>=lshift)
			out.put((ch&limitm1)|limit);
		out.put(ch);*/
		
		std::array<typename native_traits_type::char_type,sizeof(native_traits_type::char_type)*4/3+1> a;
		auto i(a.size()-1);
		for(;i<a.size()&&ch;--i)
		{
			a[i]=((ch&limitm2)|limit1);
			ch>>=lshift2;
		}
		if(a.size()<=++i)
			throw std::runtime_error("incorrect character");
		std::size_t pos(8*sizeof(native_traits_type::char_type)-(a.size()-i)-1);
		using fake_char_t = std::make_unsigned_t<typename native_traits_type::char_type>;
		fake_char_t v(0);
		v=~v;
		v>>=pos;
		v<<=pos;
		v&=~(1<<pos);
		a[i]|=v;
		a[i]&=~(1<<pos);
		ib.write(a.cbegin()+i,a.cend());
	}
	template<typename Contiguous_iterator>
	constexpr void write(Contiguous_iterator b,Contiguous_iterator e) requires standard_output_stream<T>()
	{
		auto pb(static_cast<char_type*>(static_cast<void*>(std::addressof(*b))));
		if constexpr(sizeof(*b)%sizeof(char_type))
			if((e-b)*sizeof(*b)%sizeof(char_type))
				throw std::runtime_error("incorrect write size");
		for(auto pi(pb),pe(pb+(e-b)*sizeof(*b)/sizeof(char_type));pi!=pe;++pi)
			put(traits_type::to_int_type(*pi));
	}
};


}