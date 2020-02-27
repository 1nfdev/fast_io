#include"../timer.h"
#include"../../include/fast_io_device.h"
#include"../../include/fast_io_legacy.h"

template<fast_io::stream stm,fast_io::open_mode om=fast_io::open_mode::out|fast_io::open_mode::binary,std::size_t N=10000000>
inline void test(std::string_view text)
{
	fast_io::timer tm(text);
	fast_io::basic_obuf<stm> file(text,fast_io::open_interface<om>);
	for(std::size_t i{};i!=N;++i)
		println(file,i);
}

int main()
{
	test<fast_io::c_file_unlocked>("c_file_unlocked.txt");
	test<fast_io::c_file>("c_file.txt");
	test<fast_io::posix_file>("posix.txt");
#ifdef __GLIBCXX__
	test<fast_io::stream_file>("stream_file.txt");
#endif
#if defined(__WINNT__) || defined(_MSC_VER)
	test<fast_io::win32_file>("win32.txt");
	test<fast_io::win32_file,fast_io::open_mode::out|fast_io::open_mode::binary|fast_io::open_mode::sequential_scan>("win32_scan.txt");
#endif
}