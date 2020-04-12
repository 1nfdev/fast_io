
#include"../timer.h"
#include"mmap_svector.h"
#include<vector>

int main()
{
	constexpr std::size_t N{1242};
	constexpr std::size_t M{1025};
	{
		fast_io::timer tm("mmap_svector");
		for(std::size_t i{};i!=N;++i)
		{
		fast_io::mmap_svector<std::size_t,1024> vec;
		for(std::size_t i{};i!=M;++i)
			vec.emplace_back(235);
		}
	}
	{
		fast_io::timer tm("std::vector");
		for(std::size_t i{};i!=N;++i)
		{
		std::vector<std::size_t> vec;
//		vec.reserve(M);
		for(std::size_t i{};i!=M;++i)
			vec.emplace_back(235);
		}
	}

//	println(vec.size()," ",vec.capacity());
//	for(auto const& e : vec)
//		println(e);
}