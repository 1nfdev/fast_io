#include"../../../include/fast_io.h"
#include"../../timer.h"

int main()
{
	constexpr std::size_t N{100000000};
	fast_io::timer tm("c_file");
	std::string view("Hello World\n");
	fast_io::c_file cfl("c_file.txt","wb");
	for(std::size_t i{};i!=N;++i)
		print(cfl,view);
}