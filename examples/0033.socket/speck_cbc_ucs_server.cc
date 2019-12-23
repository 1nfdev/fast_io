#include"../../include/fast_io.h"
#include"../../include/fast_io_device.h"
#include"../../include/fast_io_network.h"
#include"../../include/fast_io_crypto.h"

int main()
try
{
	//use speck cbc mode to encrypt our communication
	fast_io::server hd(2000,fast_io::sock::type::stream);
	std::array<uint8_t, 16> constexpr key {'8',u8'3',u8'3',u8'4',u8';',u8'2',u8'3',u8'4',u8'a',u8'2',u8'c',u8'4',u8']',u8'0',u8'3',u8'4'};
	std::array<uint8_t, 16> constexpr iv {'1',u8'2',u8'3',u8'4',u8'1',u8'2',u8'3',u8'4',u8'1',u8'2',u8'3',u8'4',u8'1',u8'2',u8'3',u8'4'};
	for(;;)
	try
	{
		fast_io::crypto::basic_ocbc<fast_io::acceptor_buf, fast_io::crypto::speck::speck_enc_128_128> accept(key, iv, hd);
		//fast_io::acceptor_buf accept(hd);
		std::string const str("☭🚄😀我wubi😄😊😉😍😘😚😜😝😳😁😣😢😂😭😪😥😰😩㍿🀀🀁🀂🀃🀄🀅🀆🀇🀈🀉🀊🀋🀌🀍🀎🀏");
		
//		println(accept,u8"☭🚄😀我wubi😄😊😉😍😘😚😜😝😳😁😣😢😂😭😪😥😰😩㍿🀀🀁🀂🀃🀄🀅🀆🀇🀈🀉🀊🀋🀌🀍🀎🀏");
		println(fast_io::err,u8"a client connect to this server");
		print_flush(accept,str);
	}
	catch(std::exception const& e)
	{
		println(fast_io::err,e);
	}
}
catch(std::exception const & e)
{
	println(fast_io::err,e);
	return 1;
}