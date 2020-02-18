#include"../../include/fast_io.h"
#include"../../include/fast_io_device.h"
#include"../../include/fast_io_network.h"

int main()
{
	fast_io::onative_file log("log.txt");
	for(fast_io::tcp_server hd(2000);;)
	{
		fast_io::acceptor accept(hd);
		fast_io::process proc("server_child_process.exe",{},{.in=accept,.out=accept,.err=log});
	}
}