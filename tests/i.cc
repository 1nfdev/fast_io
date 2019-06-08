#include"timer.h"
#include<fstream>
#include"../include/fast_io.h"
#include<exception>
#include<cmath>
#include<memory>
#include<cstdio>
#include"iomutex2.h"

fast_io::obuf_mutex omtx;

void f1()
{
	for(std::size_t i(100);i!=200;++i)
		fprint(omtx,"From f1: %\n",i);
}

int main()
try
{
	std::size_t constexpr N(10000000);
/*		{
	cqw::timer t("std::FILE*");
	std::unique_ptr<std::FILE,decltype(fclose)*> fp(std::fopen("cfilestar.txt","wb"),fclose);
	for(std::size_t i(0);i!=N;++i)
		fprintf(fp.get(),"%zu\n",i);
	}*/
/*	{
	cqw::timer t("obuf");
	fast_io::obuf obuf("obuf.txt");
	for(std::size_t i(0);i!=N;++i)
		(obuf<<i).put('\n');
	}*/
	{
	cqw::timer t("obuf_fprint");
	fast_io::obuf obuf("obuf_fprint.txt");
	for(std::size_t i(0);i!=N;++i)
		fprint(obuf,"%\n",i);
	}
/*	{
	cqw::timer t("obuf_mutex");
	fast_io::obuf_mutex obuf("obuf_mutex.txt");
	for(std::size_t i(0);i!=N;++i)
		print(obuf,i,fast_io::character('\n'));
	}*/
}
catch(std::exception const& e)
{
	fast_io::log lg;
	print(lg,e.what(),"\n");
	return 1;
}