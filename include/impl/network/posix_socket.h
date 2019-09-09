#pragma once

namespace fast_io
{
class socket
{
	int handle;
public:
	using native_handle_type = int;
	using char_type = char;
	template<typename ...Args>
	socket(native_interface_t,Args&& ...args):handle(::socket(std::forward<Args>(args)...))
	{
		if(handle==-1)
			throw std::system_error(errno,std::generic_category());
	}
	socket(socket_family const & family,socket_type const &type):socket(native_interface,static_cast<int>(family),static_cast<int>(type),0){}
	auto native_handle() const {return handle;}
};
/*
class server
{
	
};*/

class client
{
	
};

}