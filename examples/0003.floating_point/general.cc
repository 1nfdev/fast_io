#include"../../include/fast_io_legacy.h"


template<std::floating_point F>
inline constexpr void test(std::string_view literal,F f)
{
	println(fast_io::c_stdout,u8"literal:",literal,u8"->scientific:",fast_io::scientific(f),u8"\tfixed:",fast_io::fixed(f),u8"\tgeneral:",f);
}

int main()
{
	test("0.0",0.0);
	test("1.0",1.0);
	test("10.0",10.0);
	test("100.0",100.0);
	test("1000.0",1000.0);
	test("10000.0",10000.0);
	test("100000.0",100000.0);
	test("1000000.0",1000000.0);
	test("10000000.0",10000000.0);
	test("100000000.0",100000000.0);
	test("11000.0",11000.0);
	test("23535782139572189357.0",23535782139572189357.0);
	test("235357821395721893572.0",235357821395721893572.0);
	test("2353578213957218935723.0",2353578213957218935723.0);
	test("23535782139572189357234.0",23535782139572189357234.0);
	test("1e-2",1e-2);
	test("1e-4",1e-4);
	test("1.1e-5",1e-5);
	test("1e-6",1e-6);
	test("1e-8",1e-8);
	test("1.2241412412421421412412412",1.2241412412421421412412412);
}

/*
literal:0.0->scientific:0e+00   fixed:0 general:0
literal:1.0->scientific:1e+00   fixed:1 general:1
literal:10.0->scientific:1e+01  fixed:10        general:10
literal:100.0->scientific:1e+02 fixed:100       general:100
literal:1000.0->scientific:1e+03        fixed:1000      general:1000
literal:10000.0->scientific:1e+04       fixed:10000     general:1e+04
literal:100000.0->scientific:1e+05      fixed:100000    general:1e+05
literal:1000000.0->scientific:1e+06     fixed:1000000   general:1e+06
literal:10000000.0->scientific:1e+07    fixed:10000000  general:1e+07
literal:100000000.0->scientific:1e+08   fixed:100000000 general:1e+08
literal:11000.0->scientific:1.1e+04     fixed:11000     general:11000
literal:23535782139572189357.0->scientific:2.353578213957219e+19        fixed:23535782139572190000      general:23535782139572190000
literal:235357821395721893572.0->scientific:2.3535782139572188e+20      fixed:235357821395721880000     general:235357821395721880000
literal:2353578213957218935723.0->scientific:2.3535782139572189e+21     fixed:2353578213957218900000    general:2353578213957218900000
literal:23535782139572189357234.0->scientific:2.353578213957219e+22     fixed:23535782139572190000000   general:2.353578213957219e+22
literal:1e-2->scientific:1e-02  fixed:0.01      general:0.01
literal:1e-4->scientific:1e-04  fixed:0.0001    general:1e-04
literal:1.1e-5->scientific:1e-05        fixed:0.00001   general:1e-05
literal:1e-6->scientific:1e-06  fixed:0.000001  general:1e-06
literal:1e-8->scientific:1e-08  fixed:0.00000001        general:1e-08
literal:1.2241412412421421412412412->scientific:1.2241412412421422e+00  fixed:1.2241412412421422        general:1.2241412412421422
*/