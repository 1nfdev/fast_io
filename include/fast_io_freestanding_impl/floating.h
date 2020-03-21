#pragma once
#include"ryu/ryu.h"
//#include"grisu_exact/grisu_exact.h"
//#include"grisu_exact/grisu_exact_api.h"

namespace fast_io
{

template<buffer_input_stream input,std::floating_point T>
inline constexpr bool scan_define(input& in,T &t)
{
	if(!skip_space(in))
		return false;
	auto igen{igenerator(in)};
	t=static_cast<std::remove_cvref_t<T>>(details::ryu::input_floating<double>(begin(igen),end(igen)));
	return true;
}

template<output_stream output,std::size_t precision,std::floating_point T>
inline void print_define(output& out,manip::fixed<precision,T const> a)
{
	std::size_t constexpr reserved_size(precision+325);
	if constexpr(buffer_output_stream<output>)
	{
		auto reserved(oreserve(out,reserved_size));
		if constexpr(std::is_pointer_v<decltype(reserved)>)
		{
			if(reserved)
			{
				auto start(reserved-reserved_size);
				orelease(out,reserved-details::ryu::output_fixed<precision>(start,static_cast<double>(a.reference)));
				return;
			}
		}
		else
		{
			auto start(reserved-reserved_size);
			orelease(out,reserved-details::ryu::output_fixed<precision>(start,static_cast<double>(a.reference)));
			return;
		}
	}
	if constexpr (precision<325)
	{
		std::array<typename output::char_type,reserved_size> array;
		write(out,array.data(),details::ryu::output_fixed<precision>(array.data(),static_cast<double>(a.reference)));
	}
	else
	{
		std::basic_string<typename output::char_type> str(reserved_size);
		write(out,str.data(),details::ryu::output_fixed<precision>(str.data(),static_cast<double>(a.reference)));
	}
}

template<output_stream output,std::size_t precision,bool uppercase_e,std::floating_point T>
inline void print_define(output& out,manip::scientific<precision,uppercase_e,T const> a)
{

	std::size_t constexpr reserved_size(precision+10);
	if constexpr(buffer_output_stream<output>)
	{

		auto start(oreserve(out,reserved_size));
		if constexpr(std::is_pointer_v<decltype(start)>)
		{
			if(start)
			{
				orelease(out,details::ryu::output_fixed<precision,true,uppercase_e>(start,static_cast<double>(a.reference)));
				return;
			}
		}
		else
		{
			orelease(out,details::ryu::output_fixed<precision,true,uppercase_e>(start,static_cast<double>(a.reference)));
			return;
		}
	}
	if constexpr (precision<325)
	{
		std::array<typename output::char_type,reserved_size> array;
		write(out,array.data(),details::ryu::output_fixed<precision,true,uppercase_e>(array.data(),static_cast<double>(a.reference)));
	}
	else
	{
		std::basic_string<typename output::char_type> str(reserved_size);
		write(out,str.data(),details::ryu::output_fixed<precision,true,uppercase_e>(str.data(),static_cast<double>(a.reference)));
	}
}

template<std::floating_point T>
inline constexpr std::size_t print_reserve_size(print_reserve_type_t<manip::fixed_shortest<T>>)
{
	return 325;
}

template<std::random_access_iterator raiter,std::floating_point T,typename P>
inline raiter print_reserve_define(print_reserve_type_t<manip::fixed_shortest<T>>,raiter start,P a)
{
	return details::ryu::output_shortest<false,1>(start,static_cast<double>(a.reference));
}

template<bool uppercase_e,std::floating_point T>
inline constexpr std::size_t print_reserve_size(print_reserve_type_t<manip::scientific_shortest<uppercase_e,T>>)
{
	return 30;
}

template<std::random_access_iterator raiter,bool uppercase_e,std::floating_point T,typename P>
inline raiter print_reserve_define(print_reserve_type_t<manip::scientific_shortest<uppercase_e,T>>,raiter start,P a)
{
	return details::ryu::output_shortest<uppercase_e,2>(start,static_cast<double>(a.reference));
}

template<bool uppercase_e,std::floating_point T>
inline constexpr std::size_t print_reserve_size(print_reserve_type_t<manip::general_shortest<uppercase_e,T>>)
{
	return 30;
}

template<std::random_access_iterator raiter,bool uppercase_e,std::floating_point T,typename P>
inline raiter print_reserve_define(print_reserve_type_t<manip::general_shortest<uppercase_e,T>>,raiter start,P a)
{
	return details::ryu::output_shortest<uppercase_e>(start,static_cast<double>(a.reference));
}

template<std::floating_point T>
inline constexpr std::size_t print_reserve_size(print_reserve_type_t<manip::int_hint<T>>)
{
	return 30;
}

template<std::random_access_iterator raiter,std::floating_point T,typename P>
inline raiter print_reserve_define(print_reserve_type_t<manip::int_hint<T>>,raiter start,P a)
{
	return details::ryu::output_shortest<false,0,true>(start,static_cast<double>(a.reference));
}

template<std::floating_point T>
inline constexpr std::size_t print_reserve_size(print_reserve_type_t<T>)
{
	return 30;
}

template<std::random_access_iterator raiter,std::floating_point T>
inline raiter print_reserve_define(print_reserve_type_t<T>,raiter start,T a)
{
//	if constexpr(sizeof(double)<sizeof(T))
		return details::ryu::output_shortest<false>(start,static_cast<double>(a));
//	else
//		return details::ryu::output_shortest<false>(start,a);
}


}
