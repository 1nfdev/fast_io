#pragma once

namespace fast_io
{
	
namespace details
{

template<standard_output_stream output,std::floating_point T>
inline bool constexpr output_inf(output& out,T e)
{
	if(e==std::numeric_limits<T>::signaling_NaN()||e==std::numeric_limits<T>::quiet_NaN())
	{
		print(out,"nan");
		return true;		
	}
	else if(e==std::numeric_limits<T>::infinity())
	{
		print(out,"inf");
		return true;
	}
	return false;
}

}

template<standard_output_stream output,std::floating_point T>
inline void print(output& out,details::fixed<T const> a)
{
	auto e(a.reference);
	if(e<0)
	{
		e=-e;
		out.put('-');
	}
	if(details::output_inf(out,e))
		return;
	auto u(floor(e));
	e-=u;
	if(a.precision)
	{
		auto p(e*pow(static_cast<T>(10),a.precision+1));
		auto pround(round(p));
		auto mdg(fmod(p,10)),ptg(floor(p/10));
		if(mdg<5||(mdg==5&&(fmod(ptg,2)==0||floor(p)!=p)))
		{
//			std::cout<<"47 here: "<<mdg<<"\n";
			std::basic_string<typename output::char_type> bas;
			auto pu(u);
			do
			{
				bas.push_back(static_cast<typename output::char_type>(fmod(pu,10)+48));
			}
			while((pu=floor(pu/10)));
			std::reverse(bas.begin(),bas.end());
			out.write(bas.cbegin(),bas.cend());
			out.put('.');
			bas.clear();
			auto pv(ptg);
			do
			{
				bas.push_back(static_cast<typename output::char_type>(fmod(pv,10)+48));
			}
			while((pv=floor(pv/10))&&bas.size()<=a.precision);
			std::reverse(bas.begin(),bas.end());
			for(std::size_t i(bas.size());i<a.precision;++i)
				out.put(48);
			print(out,bas);
		}
		else
		{
//			std::cout<<"72 here: \n";
			++ptg;
			if(pround+1==round(1/pow(static_cast<T>(10),a.precision)))
				++u;				
			std::basic_string<typename output::char_type> bas;
			do
			{
				bas.push_back(static_cast<typename output::char_type>(fmod(u,10)+48));
			}
			while((u=floor(u/10)));
			std::reverse(bas.begin(),bas.end());
			print(out,bas);
			bas.clear();
			out.put('.');
			do
			{
				if(a.precision<=bas.size())
					break;
				bas.push_back(static_cast<typename output::char_type>(fmod(ptg,10)+48));
			}
			while((ptg=floor(ptg/10)));
			std::reverse(bas.begin(),bas.end());
			for(std::size_t i(bas.size());i<a.precision;++i)
				out.put('0');
			print(out,bas);
		}
	}
	else
		print(out,u);
}

template<standard_output_stream output,std::floating_point T>
inline void print(output& out,details::scientific<T const> a)
{
	auto e(a.reference);
	if(e==0)	//if e==0 then log10 is UNDEFINED
	{
		out.put('0');
		if(a.precision)
		{
			out.put('.');
			for(std::size_t i(0);i!=a.precision;++i)
				out.put('0');
		}
		print(out,"e0");
		return;
	}
	if(e<0)
	{
		e=-e;
		out.put('-');
	}
	if(details::output_inf(out,e))
		return;
	auto x(std::floor(std::log10(e)));
	print(out,fixed(e*std::pow(10,-x),a.precision));
	out.put('e');
	if(x<0)
	{
		out.put('-');
		x=-x;
	}
	print(out,static_cast<std::uint64_t>(x));
}

template<standard_output_stream output,std::floating_point T>
inline void print(output& out,details::floating_point_default<T const> a)
{
	auto e(a.reference);
	if(e==0)	//if e==0 then log10 is UNDEFINED
	{
		out.put('0');
		return;
	}
	if(e<0)
	{
		e=-e;
		out.put('-');
	}
	if(details::output_inf(out,e))
		return;
	auto x(std::floor(std::log10(e)));
	{
	auto fix(std::fabs(x)<=a.precision);
	basic_ostring<std::basic_string<typename output::char_type>> bas;
	if(fix)
		print(bas,fixed(e,a.precision));
	else
		print(bas,fixed(e*std::pow(10,-x),a.precision));
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

template<standard_output_stream soutp,std::floating_point T>
inline void print(soutp &output,T const& p)
{
	print(output,floating_point_default(p,6));
}

template<standard_input_stream input,std::floating_point T>
inline constexpr void scan(input& in,T &t)
{
	decltype(in.get()) ch;
	bool negative(false);
	bool phase2(false);
	for(;;)
	{
		ch=in.get();
		if(48<=ch&&ch<=57)
		{
			t=ch-48;
			break;
		}
		else
		{
			if(ch=='-')
			{
				ch=in.get();
				if(48<=ch&&ch<=57)
				{
					negative=true;
					t=ch-48;
					break;
				}
				else if(ch=='.')
				{
					negative=true;
					phase2=true;
					t=0;
					break;
				}
			}
			else if(ch=='.')
			{
				ch=in.get();
				if(48<=ch&&ch<=57)
				{
					phase2=true;
					t=0;
					break;
				}
			}
		}
	}
	if(!phase2)
	{
		while(true)
		{
			auto try_ch(in.try_get());
			if(48<=try_ch.first&&try_ch.first<=57)
				t=t*10+try_ch.first-48;
			else if(try_ch.first=='.')
			{
				phase2 = true;
				break;
			}
			else if(try_ch.first=='e'||try_ch.first=='E')
				break;
			else
			{
				if(negative)
					t=-t;
				return;
			}
		}
	}
	if(phase2)
	{
		T current(10);
		for(;;current*=10)
		{
			auto try_ch(in.try_get());
			if(48<=try_ch.first&&try_ch.first<=57)
				t+=(try_ch.first-48)/current;
			else if(try_ch.first=='e'||try_ch.first=='E')
				break;
			else
			{
				if(negative)
					t=-t;
				return;
			}
		}
	}
	std::ptrdiff_t p;
	scan(in,p);
	t*=std::pow(10,p);
	if(negative)
		t=-t;
}

template<standard_output_stream output,std::integral T>
inline void print(output& out,details::floating_point_default<T const> a)
{
	print(out,a.reference);
}

template<standard_output_stream output,std::integral T>
inline void print(output& out,details::fixed<T const> a)
{
	print(out,a.reference);
	if(a.precision)
	{
		out.put('.');
		for(std::size_t i(0);i!=a.precision;++i)
			out.put('0');
	}
}

template<standard_output_stream output,std::integral T>
inline void print(output& out,details::scientific<T const> sc)
{
	print(out,scientific(static_cast<long double>(sc.reference),sc.precision));
}

}
