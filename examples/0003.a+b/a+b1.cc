#include"../../include/fast_io.h"
#include<cstddef>

int main()
{
	println(fast_io::out,u8"Please input 2 numbers");
	std::size_t a,b;
	scan(fast_io::in,a,b);
	println(fast_io::out,u8"sum of a+b = ",a+b);
}