#include"../../include/fast_io_hosted.h"
#include"../../include/fast_io_device.h"

int main()
{
	fast_io::onative_file log("log.txt");
	for(std::size_t i(2000);i!=3500;++i)
		fast_io::process proc("./server",{fast_io::concat(i)},{.err=log});
}