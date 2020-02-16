#include"../../include/fast_io.h"

int main()
try
{
	fast_io::native_pipe pipe;
	fast_io::posix_waiting_process process;
	if(is_child(process))
	{
		pipe.in().close();
		fast_io::posix_exec("/usr/local/bin/gcc",{"-v"});
	}
	pipe.out().close();
	transmit(fast_io::out,pipe);
}
catch(std::exception const& e)
{
	println(fast_io::err,e);
	return 1;
}