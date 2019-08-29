#pragma once

#include"concept.h"
#include"stringbuf.h"
#include"rd_type.h"
#include<type_traits>
#include<cmath>

namespace fast_io
{
namespace details
{
template<typename T>
struct char_view_t
{
	T& reference;
};

template<Integral T>
struct unsigned_view_t
{
	T& reference;
};

template<Integral T>
struct signed_view_t
{
	T& reference;
};

template<typename T>
struct setw_t
{
	std::size_t width;
	T& reference;
};

template<typename T,Integral char_type>
struct setw_fill_t
{
	std::size_t width;
	T& reference;
	char_type ch;
};
template<typename T>
struct fixed
{
public:
	T& reference;
	std::size_t precision;
};
template<typename T>
struct scientific
{
public:
	T& reference;
	std::size_t precision;
};
template<typename T>
struct floating_point_default
{
public:
	T& reference;
	std::size_t precision;
};

}
template<Integral T>
inline constexpr details::char_view_t<T> char_view(T& ch)
{
	return {ch};
}
template<Integral T>
inline constexpr details::char_view_t<T const> char_view(T const& ch)
{
	return {ch};
}

template<Integral T>
inline constexpr T& unsigned_view(T& integral)
{
	return reinterpret_cast<std::make_unsigned_t<T>&>(integral);
}

template<Integral T>
inline constexpr details::signed_view_t<T> signed_view(T& integral)
{
	return reinterpret_cast<std::make_signed_t<T>&>(integral);
}

template<Integral T>
inline constexpr details::unsigned_view_t<T const> unsigned_view(T const& integral)
{
	return {integral};
}

template<Integral T>
inline constexpr details::signed_view_t<T const> signed_view(T const& integral)
{
	return {integral};
}

//	template<template T>
//	requires requires std::is_floating_point_v<T>
template<typename T>
inline constexpr details::fixed<T const> fixed(T const &f,std::size_t precision)
{
	return {f,precision};
}
template<typename T>
inline constexpr details::scientific<T const> scientific(T const &f,std::size_t precision)
{
	return {f,precision};
}
template<typename T>
inline constexpr details::floating_point_default<T const> floating_point_default(T const &f,std::size_t precision)
{
	return {f,precision};
}

inline void scan(standard_input_stream& in,details::char_view_t<Integral> a)
{
	a.reference = in.get();
}

inline void print(standard_output_stream& out,details::char_view_t<Integral> a)
{
	out.put(a.reference);
}

template<typename T>
inline details::setw_t<T const> setw(std::size_t width,T const&t)
{
	return {width,t};
}

template<typename T,Integral char_type>
inline constexpr details::setw_fill_t<T const,char_type> setw(std::size_t width,T const&t,char_type ch)
{
	return {width,t,ch};
}

template<standard_output_stream output>
inline void print(output& out,details::setw_fill_t<auto,Integral> a)
{
	basic_ostring<std::basic_string<typename output::char_type>> bas;
	print(bas,a.reference);
	for(std::size_t i(bas.str().size());i<a.width;++i)
		out.put(a.ch);
	print(out,bas.str());
}

template<standard_output_stream output>
inline void print(output& out,details::setw_t<auto> a)
{
	basic_ostring<std::basic_string<typename output::char_type>> bas;
	print(bas,a.reference);
	for(std::size_t i(bas.str().size());i<a.width;++i)
		out.put(' ');
	print(out,bas.str());
}
/*
template<typename T>
inline constexpr void print(standard_output_stream& out,details::unsigned_view_t<T> a)
{
	print(out,static_cast<std::make_unsigned_t<T>>(a.reference));
}

template<typename T>
inline constexpr void scan(standard_input_stream& in,details::unsigned_view_t<T> a)
{
	scan(in,reinterpret_cast<std::make_unsigned_t<T>&>(a.reference));
}

template<typename T>
inline constexpr void print(standard_output_stream& out,details::signed_view_t<T> a)
{
	print(out,static_cast<std::make_signed_t<T>>(a.reference));
}

template<typename T>
inline constexpr void scan(standard_input_stream& in,details::signed_view_t<T> a)
{
	scan(in,reinterpret_cast<std::make_signed_t<T>&>(a.reference));
}*/

namespace details
{
template<std::size_t v>
inline auto constexpr compiler_time_10_exp_calculation()
{
	std::array<std::uint64_t,v> a{1};
	for(std::size_t i(1);i<a.size();++i)
		a[i]=a[i-1]*10;
	return a;
}

inline auto constexpr exp10_array(compiler_time_10_exp_calculation<20>());
inline auto constexpr log2_minus_10(-std::log2(10));

}

template<typename T>
inline void print(standard_output_stream& out,details::fixed<T> a)
{
	auto e(a.reference);
	if(e<0)
	{
		e=-e;
		out.put('-');
	}
	std::uint_fast64_t u(e);
	print(out,u);
	e-=u;
	if(a.precision)
	{
		out.put('.');
		auto p(static_cast<decltype(u)>(e*details::exp10_array.at(a.precision+1)));
		auto md(p%10),pt(p/10);
		if(md<5||(md==5&&!(pt&1)))
			print(out,setw(a.precision,pt,'0'));
		else
			print(out,setw(a.precision,pt+1,'0'));
	}
}

template<typename T>
void print(standard_output_stream& out,details::scientific<T> a)
{
	auto e(a.reference);
	if(e<0)
	{
		e=-e;
		out.put('-');
	}
	auto x(std::floor(std::log10(e)));
	if(0<x)
		++x;
	print(out,fixed(e*std::exp2(x*details::log2_minus_10),a.precision));
	if(x==0)
		return;
	out.put('e');
	if(x<0)
	{
		out.put('-');
		print(out,static_cast<std::uint64_t>(-x));
	}
	print(out,static_cast<std::uint64_t>(x));
}

template<standard_output_stream output,typename T>
inline void print(output& out,details::floating_point_default<T> a)
{
	auto e(a.reference);
	if(e<0)
	{
		e=-e;
		out.put('-');
	}
	auto x(std::floor(std::log10(e)));
	if(0<x)
		++x;
	{
	auto fix(std::fabs(x)<=a.precision);
	basic_ostring<std::basic_string<typename output::char_type>> bas;
	if(fix)
		print(bas,fixed(e,a.precision));
	else
		print(bas,fixed(e*std::exp2(x*details::log2_minus_10),a.precision));
	auto& str(bas.str());
	if(str.find('.')!=std::string::npos)
	{
		for(;!str.empty()&&str.back()=='0';str.pop_back());
		if(!str.empty()&&str.back()=='.')
			str.pop_back();
		print(out,str);
	}
	if(fix)
		return;
	}
	if(x==0)
		return;
	out.put('e');
	if(x<0)
	{
		out.put('-');
		x=-x;
	}
	print(out,static_cast<std::uint64_t>(x));
}

/*
inline void print(standard_output_stream& out,double v)
{
	print(out,floating_point_default(v,14));
}*/

}