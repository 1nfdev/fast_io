#pragma once

namespace fast_io
{

template<buffer_input_stream input>
struct basic_input_buffer_iterator
{
	input *ptr{};
};

namespace details
{
template<buffer_input_stream input>
constexpr basic_input_buffer_iterator<input>& deal_with_underflow(basic_input_buffer_iterator<input>& gen)
{
	if(!underflow(ref))[[unlikely]]
	{
		gen.ptr={};
		return gen;
	}
	++ibuffer_gbegin(ref);
	return gen;
}
}

template<buffer_input_stream input>
inline constexpr basic_input_buffer_iterator<input>& operator++(basic_input_buffer_iterator<input>& gen)
{
	auto& ref(*gen.ptr);
	decltype(auto) gi{ibuffer_gbegin(ref)};
	if(gi==ibuffer_gend(ref))[[unlikely]]
		return details::deal_with_underflow(gen);
	++gi;
	return gen;
}

template<buffer_input_stream input>
inline constexpr void operator++(basic_input_buffer_iterator<input>& gen,int)
{
	static_cast<void>(operator++(gen));
}

template<buffer_input_stream input>
inline constexpr bool operator!=(basic_input_buffer_iterator<input> a, std::default_sentinel_t)
{
	return a.ptr;
}

template<buffer_input_stream input>
inline constexpr bool operator==(basic_input_buffer_iterator<input> a, std::default_sentinel_t)
{
	return a.ptr==nullptr;
}

template<buffer_input_stream input>
inline constexpr bool operator!=(std::default_sentinel_t,basic_input_buffer_iterator<input> a)
{
	return a.ptr;
}

template<buffer_input_stream input>
inline constexpr bool operator==(std::default_sentinel_t,basic_input_buffer_iterator<input> a)
{
	return a.ptr==nullptr;
}

template<buffer_input_stream input>
struct basic_input_buffer_generator
{
	input *ptr{};
};

template<buffer_input_stream input>
inline constexpr basic_input_buffer_iterator<input> begin(basic_input_buffer_generator<input> a)
{
	return {a.ptr};
}

template<buffer_input_stream input>
inline constexpr std::default_sentinel_t end(basic_input_buffer_generator<input> a)
{
	return {};
}

template<buffer_input_stream input>
inline constexpr basic_input_buffer_generator<input> igenerator(input& in)
{
	if(ibuffer_gbegin(in)==ibuffer_gend(in))[[unlikely]]
	{
		if(!underflow(in))[[unlikely]]
			return {};
	}
	return {std::addressof(in)};
}

}