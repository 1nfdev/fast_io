#include"timer.h"
#include<fstream>
#include"../../include/fast_io.h"
#include<exception>
#include<cmath>
#include<memory>
#include<cstdio>

int main()
try
{
	std::size_t constexpr N(10000000);
	{
	cqw::timer t("std::FILE*");
	std::unique_ptr<std::FILE,decltype(fclose)*> fp(std::fopen("cfilestar.txt","wb"),fclose);
	for(std::size_t i(0);i!=N;++i)
		fprintf(fp.get(),"%zu\n",i);
	}
	{
	cqw::timer t("std::FILE* with 1048576 buffer size + _IOFBF (Full buffering) tag");
	std::unique_ptr<std::FILE,decltype(fclose)*> fp(std::fopen("cfilestar_with_large_buffer.txt","wb"),fclose);
	auto buffer(std::make_unique<char[]>(1048576));
	setvbuf(fp.get(),buffer.get(),_IOFBF,1048576);
	for(std::size_t i(0);i!=N;++i)
		fprintf(fp.get(),"%zu\n",i);
	}
	{
	cqw::timer t("std::ofstream");
	std::ofstream fout("ofstream.txt",std::ofstream::binary);
	for(std::size_t i(0);i!=N;++i)
		fout<<i<<'\n';
	}
	{
	cqw::timer t("std::ofstream");
	std::ofstream fout("ofstream.txt",std::ofstream::binary);
	for(std::size_t i(0);i!=N;++i)
		fout<<i<<'\n';
	}
	{
	cqw::timer t("std::ofstream with tricks");
	std::ofstream fout("ofstream_tricks.txt",std::ofstream::binary);
	auto &rdbuf(*fout.rdbuf());
	for(std::size_t i(0);i!=N;++i)
	{
		fout<<i;
		rdbuf.sputc('\n');
	}
	}
	}
	{
	cqw::timer t("obuf");
	fast_io::obuf obuf("obuf.txt",fast_io::open::interface<fast_io::open::binary>);
	for(std::size_t i(0);i!=N;++i)
		println(obuf,i);
	}
	{
	cqw::timer t("dynamic obuf");
	fast_io::dynamic_standard_output_stream dobuf(std::in_place_type<fast_io::obuf>,"dynamic_obuf.txt",fast_io::open::interface<fast_io::open::binary>);
	for(std::size_t i(0);i!=N;++i)
		println(dobuf,i);
	}
	{
	cqw::timer t("iobuf_dynamic system_file");
	fast_io::iobuf_dynamic dobuf(std::in_place_type<fast_io::osystem_file>,"iobuf_dynamic_system_file.txt",fast_io::open::interface<fast_io::open::binary>);
	for(std::size_t i(0);i!=N;++i)
		println(dobuf,i);
	}
	{
	cqw::timer t("obuf_mutex");
	fast_io::obuf_mutex obuf("obuf_mutex.txt",fast_io::open::interface<fast_io::open::binary>);
	for(std::size_t i(0);i!=N;++i)
		println(obuf,i);
	}
	
}
catch(std::exception const& e)
{
	println(fast_io::err,e);
	return 1;
}