#pragma once

namespace fast_io
{
namespace manip
{
template<std::integral ch_type,typename T>
struct lcv
{
	using manip_tag = manip_tag_t;
	basic_lconv_storage<ch_type> const& storage;
	T reference;
};
}

namespace details
{

template<std::integral char_type>
inline constexpr auto process_lcv_grouping(std::basic_string_view<char_type> const& grouping,
	char_type* str_first,char_type* str_last,
	char_type* buffer_iter,char_type seperator)
{
	using unsigned_char_type =std::make_unsigned_t<char_type>;
	auto gp_cendm1{grouping.cend()-1};
	for(auto iter(grouping.cbegin());;)
	{
		unsigned_char_type gp{static_cast<unsigned_char_type>(*iter)};
		if(gp)[[likely]]
		{
			if(str_last-gp<str_first)[[unlikely]]
				break;
			if(std::is_constant_evaluated())
				std::copy_n(str_last-=gp,gp,buffer_iter-=gp);
			else
				memcpy(buffer_iter-=gp,str_last-=gp,gp*sizeof(char_type));
			*--buffer_iter=seperator;
		}
		if(iter!=gp_cendm1)[[likely]]
			++iter;
	}
	if(std::size_t const sz(str_last-str_first);sz)
	{
		if(std::is_constant_evaluated())
			std::copy_n(str_first,sz,buffer_iter-=sz);
		else
			memcpy(buffer_iter-=sz,str_first,sz*sizeof(char_type));
	}
	else
		++buffer_iter;
	return buffer_iter;
}

template<std::integral int_type,char8_t base>
constexpr std::size_t cal_lcv_integer_output_size()
{
	constexpr std::size_t rsv_size{(cal_max_uint_size<std::make_unsigned_t<int_type>,base>()+2)<<1};
	return rsv_size;
}

template<std::integral char_type,char8_t base,bool uppercase,std::contiguous_iterator caiter,typename T,std::integral int_type>
inline constexpr auto process_lcv_integer_output(caiter outiter,T const& storage,int_type value)
{
	auto grouping{storage.grouping()};
	if(grouping.empty())
		return process_integer_output<base,uppercase>(outiter,value);
	std::array<char_type,(details::cal_max_uint_size<std::make_unsigned_t<int_type>,base>()+1)<<1> str;
	auto str_iter(details::process_integer_output<base,uppercase,true>(str.data(),value));
	constexpr std::size_t buffer_size{cal_lcv_integer_output_size<int_type,base>()};
	auto buffer_iter{process_lcv_grouping(grouping,str.data(),str_iter,outiter+buffer_size,storage.thousands_sep)};
	if constexpr(std::signed_integral<int_type>)
	{
		if(value<0)
			*--buffer_iter=u8'-';
	}
	return details::my_copy(buffer_iter,outiter+buffer_size,outiter);
}

}
template<std::integral ch_type,typename T>
inline constexpr manip::lcv<ch_type,T&> lcv(basic_lconv_storage<ch_type> const& t,T&& f){return {t,f};}

template<std::integral ch_type,std::integral int_type>
inline constexpr std::size_t print_reserve_size(print_reserve_type_t<manip::lcv<ch_type,int_type&>>)
{
	return details::cal_lcv_integer_output_size<int_type,10>();
}

template<std::contiguous_iterator caiter,std::integral char_type,std::integral int_type>
inline constexpr caiter print_reserve_define(print_reserve_type_t<manip::lcv<char_type,int_type&>>,caiter outiter,auto ref)
{
	return details::process_lcv_integer_output<char_type,10,false>(outiter,ref.storage,ref.reference);
}

template<std::integral ch_type,char8_t base,bool uppercase,std::integral int_type>
inline constexpr std::size_t print_reserve_size(print_reserve_type_t<manip::lcv<ch_type,manip::base_t<base,uppercase,int_type>&>>)
{
	return details::cal_lcv_integer_output_size<int_type,base>();
}

template<std::contiguous_iterator caiter,std::integral char_type,char8_t base,bool uppercase,std::integral int_type>
inline constexpr caiter print_reserve_define(print_reserve_type_t<manip::lcv<char_type,manip::base_t<base,uppercase,int_type>&>>,caiter outiter,auto ref)
{
	return details::process_lcv_integer_output<char_type,base,uppercase>(outiter,ref.storage,ref.reference.reference);
}


}