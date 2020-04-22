#pragma once
#include<thread>

namespace fast_io
{

namespace manip
{

template<typename Func>
struct parallel_counter
{
	Func callback;
	std::size_t count{};
	std::size_t chars_per_element{};
};
template<typename T,typename Func>
struct parallel
{
	T& reference;
	[[no_unique_address]] Func callback;
};

}

template<typename Func>
inline constexpr manip::parallel_counter<Func> parrallel_counter(Func func,std::size_t count,std::size_t chars_per_element){return {func,count,chars_per_element};}

template<std::ranges::random_access_range T>
inline constexpr manip::parallel<T,void> parrallel(T& r){return {r};}
template<std::ranges::random_access_range T,typename Func>
inline constexpr manip::parallel<T,void> parrallel(T& r,Func callback){return {r,callback};}

namespace details
{
template<std::integral ch_type,typename Func>
inline constexpr void print_out(fast_io::basic_ospan<std::span<ch_type>>* osp,Func func,std::size_t start_number,std::size_t stop_number)
{
	fast_io::basic_ospan<std::span<ch_type>> bas{*osp};
	func(bas,start_number,stop_number);
	*osp=bas;
}

template<std::integral ch_type>
struct span_raii
{
	using ospan_type = fast_io::basic_ospan<std::span<ch_type>>;
	ospan_type osp;
//	constexpr span_raii()=default;
	template<typename... Args>
	requires std::constructible_from<ospan_type,Args...>
	constexpr span_raii(Args&& ...args) noexcept:osp(std::forward<Args>(args)...){}
	constexpr span_raii(span_raii const&)=delete;
	constexpr span_raii(span_raii&& other) noexcept:osp(other.osp)
	{
		other.osp.span()={};
	}
	constexpr span_raii& operator=(span_raii const&)=delete;
	constexpr span_raii& operator=(span_raii&& other) noexcept
	{
		if(std::addressof(other)==this)
			return *this;
		if(osp.span().data())[[likely]]
			delete[] osp.data();	
		osp.span()=other.osp.span();
		other.osp.span()={};
		return *this;
	}

	constexpr ~span_raii()
	{
		if(osp.span().data())[[likely]]
			delete[] osp.span().data();
	}
};

template<output_stream stm,typename Func>
inline constexpr void parrallel_details(stm& output,std::size_t count,std::size_t chars_per_element,Func func)
{
	using char_type = typename stm::char_type;
	std::size_t thread_number{std::thread::hardware_concurrency()};

	if(thread_number<2)[[unlikely]]
	{
		if constexpr(reserve_output_stream<stm>)
		{
			func(output,0,count);
		}
		else
		{
			std::size_t const total_chars{count*chars_per_element};
			span_raii<char_type> osp{new char_type[total_chars],total_chars};
			func(osp.osp,0,count);
			write(output,osp.osp.span().data(),osp.osp.span().data()+osize(osp.osp));
		}
		return;
	}

	std::size_t range{count/thread_number};
	std::size_t const module{count-range*thread_number};
	std::size_t const allocation_size{range*chars_per_element};

	std::vector<details::span_raii<char_type>> thread_result;
	thread_result.reserve(thread_number);
	{
	std::vector<std::jthread> jth;
	jth.reserve(thread_number);
	std::size_t const offset{(range<<1)+module};
	std::size_t const offset_allocation_size{(range+module)*chars_per_element};
	jth.emplace_back(print_out<char_type,Func>,std::addressof(thread_result.emplace_back(new char_type[offset_allocation_size],offset_allocation_size).osp),
		func,range,offset);
	for(std::size_t i{2};i<thread_number;++i)
		jth.emplace_back(print_out<char_type,Func>,std::addressof(thread_result.emplace_back(new char_type[allocation_size],allocation_size).osp),
		func,offset+(i-2)*range,offset+(i-1)*range);
	if constexpr(reserve_output_stream<stm>)
		func(output,0,range);
	else
	{
		span_raii<char_type> osp{new char_type[allocation_size],allocation_size};
		func(osp.osp,0,range);
		write(output,osp.osp.span().data(),osp.osp.span().data()+osize(osp.osp));
	}
	}
	for(auto& e : thread_result)
		write(output,e.osp.span().data(),e.osp.span().data()+osize(e.osp));
}

}

template<output_stream stm,typename Func>
inline constexpr void print_define(stm& output,manip::parallel_counter<Func> ref)
{
	details::parrallel_details(output,ref.count,ref.chars_per_element,ref.callback);
}


template<output_stream stm,std::ranges::random_access_range R,typename Func>
inline constexpr void print_define(stm& output,manip::parallel<R,Func> ref)
{
	using char_type = typename stm::char_type;
	constexpr std::size_t sz{print_reserve_size(print_reserve_type<std::remove_cvref_t<
	std::conditional_t<std::same_as<Func,void>,
	std::ranges::range_value_t<R>,decltype(ref.callback(*std::ranges::begin(ref.reference)))>>>())};
	
	details::parrallel_details(output,std::ranges::size(ref.reference),sz,
		[&](reserve_output_stream auto& output,std::size_t start_number,std::size_t stop_number)
	{
		auto iter{std::ranges::begin(ref.reference)+start_number};
		auto ed{std::ranges::end(ref.reference)+stop_number};
		for(;iter!=ed;++iter)
			if constexpr(std::same_as<Func,void>)
				print(output,*iter);
			else
				print(output,ref.callback(*iter));
	});
}

}