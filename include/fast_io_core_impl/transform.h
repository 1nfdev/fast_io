#pragma once

namespace fast_io
{


template<output_stream output,typename func,std::integral ch_type = typename output::char_type,std::size_t buffer_size=4096,bool randomaccess=false>
class otransform
{
public:
	using native_handle_type = output; 
	using transform_function_type = func;
	using char_type = ch_type;
	std::pair<native_handle_type,transform_function_type> handle;
	std::size_t position{};
	std::array<char_type,buffer_size> buffer;
	auto& native_handle()
	{
		return handle.first;
	}
	auto& transform_function()
	{
		return handle.second;
	}
private:
	void close()
	{
		try
		{
			if(position!=static_cast<std::size_t>(-1))
				handle.second.write_proxy(handle.first,buffer.data(),buffer.data()+position);
		}
		catch(...){}
	}
public:
	template<typename... Args>
	requires std::constructible_from<std::pair<native_handle_type,transform_function_type>,Args...>
	constexpr otransform(Args&& ...args):handle(std::forward<Args>(args)...){}
	otransform(otransform const&) = default;
	otransform& operator=(otransform const&) = default;
	otransform(otransform&& other) noexcept:handle(std::move(other.handle)),
			position(std::move(other.position)),
			buffer(std::move(other.buffer))
	{
		other.position=static_cast<std::size_t>(-1);
	}
	otransform& operator=(otransform&& other) noexcept
	{
		if(std::addressof(other)!=this)
		{
			close();
			handle=std::move(other.handle);
			position=std::move(other.position);
			buffer=std::move(other.buffer);
			other.position=static_cast<std::size_t>(-1);
		}
		return *this;
	}
	~otransform()
	{
		close();
	}
};


template<output_stream output,typename func,std::integral ch_type = typename output::char_type,std::size_t sz=4096,bool rac=false>
requires std::is_default_constructible_v<func>
class otransform_function_default_construct:public otransform<output,func,ch_type,sz,rac>
{
public:
	using native_handle_type = output; 
	using transform_function_type = func;
	using char_type = ch_type;
	template<typename... Args>
	requires std::constructible_from<native_handle_type,Args...>
	otransform_function_default_construct(Args&& ...args):otransform<output,func>(std::piecewise_construct,
				std::forward_as_tuple(std::forward<Args>(args)...),std::forward_as_tuple()){}
};
namespace details
{

template<typename T,std::contiguous_iterator Iter>
inline constexpr void otransform_write(T& ob,Iter cbegin,Iter cend)
{
	std::size_t diff(cend-cbegin);
	if(ob.buffer.size()<=ob.position+diff)[[unlikely]]
	{
		cbegin=std::copy_n(cbegin,ob.buffer.size()-ob.position,ob.buffer.data()+ob.position);
		ob.handle.second.write_proxy(ob.handle.first,ob.buffer.data(),ob.buffer.data()+ob.buffer.size());
		std::size_t remain_length(diff - (ob.buffer.size()-ob.position));
		
		while (remain_length)
		{
			std::size_t out_length(std::min(ob.buffer.size(), remain_length));
			cbegin=std::copy_n(cbegin,out_length,ob.buffer.data());
			if (out_length == ob.buffer.size())[[unlikely]]
			{
				ob.handle.second.write_proxy(ob.handle.first,ob.buffer.data(),ob.buffer.data()+ob.buffer.size());
				remain_length -= out_length;
			}
			else
				break;
		}
		ob.position=remain_length;
		/*for(ob.handle.second.write_proxy(ob.handle.first,ob.buffer.data(),ob.buffer.data()+ob.buffer.size());
			(cbegin=std::copy_n(cbegin,ob.buffer.size(),ob.buffer.data()))!=cend;
			ob.handle.second.write_proxy(ob.handle.first,ob.buffer.data(),ob.buffer.data()+ob.buffer.size()));*/

		return;
	}
	std::copy_n(cbegin,diff,ob.buffer.data()+ob.position);
	ob.position+=diff;
}

}

template<output_stream Ohandler,typename func,std::integral ch_type,std::contiguous_iterator Iter,std::size_t sz,bool rac>
inline constexpr void write(otransform<Ohandler,func,ch_type,sz,rac>& ob,Iter cbegini,Iter cendi)
{
	using char_type = typename otransform<Ohandler,func,ch_type,sz,rac>::char_type;
	if constexpr(std::same_as<char_type,typename std::iterator_traits<Iter>::value_type>)
		details::otransform_write(ob,std::to_address(cbegini),std::to_address(cendi));
	else
		details::otransform_write(ob,reinterpret_cast<std::byte const*>(std::to_address(cbegini)),
					reinterpret_cast<std::byte const*>(std::to_address(cendi)));
}

template<output_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac>
inline constexpr void flush(otransform<Ohandler,func,ch_type,sz,rac>& ob)
{
	if(ob.position!=static_cast<std::size_t>(-1))
		ob.handle.second.write_proxy(ob.handle.first,ob.buffer.data(),ob.buffer.data()+ob.position);
}

template<buffer_input_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac>
inline constexpr decltype(auto) iflush(otransform<Ohandler,func,ch_type,sz,rac>& out)
{
	return iflush(*out);
}

template<buffer_input_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac>
inline constexpr decltype(auto) begin(otransform<Ohandler,func,ch_type,sz,rac>& out)
{
	return begin(out.native_handle());
}

template<buffer_input_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac>
inline constexpr decltype(auto) end(otransform<Ohandler,func,ch_type,sz,rac>& out)
{
	return end(out.native_handle());
}

template<buffer_input_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac>
inline constexpr otransform<Ohandler,func>& operator++(otransform<Ohandler,func,ch_type,sz,rac>& out)
{
	operator++(out.native_handle());
	return out;
}

template<buffer_input_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac,std::integral I>
inline constexpr otransform<Ohandler,func>& operator+=(otransform<Ohandler,func,ch_type,sz,rac>& out,I i)
{
	operator+=(out.native_handle(),i);
	return out;
}

template<output_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac,std::integral I>
[[nodiscard]] inline constexpr auto oreserve(otransform<Ohandler,func,ch_type,sz,rac>& ob,I i) -> decltype(ob.buffer.data())
{
	if(ob.buffer.size()<=ob.position+i)[[unlikely]]
		return nullptr;
	ob.position+=i;
	return ob.buffer.data()+ob.position;
}

template<output_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac,std::integral I>
inline constexpr void orelease(otransform<Ohandler,func,ch_type,sz,rac>& ob,I i)
{
	ob.position-=i;
}

template<output_stream Ohandler,typename func,std::integral ch_type,std::size_t sz,bool rac>
inline constexpr void put(otransform<Ohandler,func,ch_type,sz,rac>& ob,typename otransform<Ohandler,func,ch_type,sz,rac>::char_type ch)
{
	if(ob.position==ob.buffer.size())[[unlikely]]		//buffer full
	{
		ob.handle.second.write_proxy(ob.handle.first,ob.buffer.data(),ob.buffer.data()+ob.buffer.size());
		ob.position=1;
		ob.buffer.front()=ch;
		return;//no flow dependency any more
	}
	ob.buffer[ob.position]=ch;
	++ob.position;
}

template<output_stream output,typename func,std::integral ch_type,std::size_t sz,bool rac,typename... Args>
requires (random_access_stream<output>&&rac)
inline constexpr auto seek(otransform<output,func,ch_type,sz,rac>& in,Args&& ...args)
{
	in.position={};
	return in.seek_proxy(in.handle.first,std::forward<Args>(args)...);
}

}