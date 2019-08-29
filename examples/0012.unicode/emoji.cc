#include"../../include/fast_io.h"

int main()
{
	using namespace fast_io;
	fast_io::obuf ob("emoji.txt");
	fast_io::unicode_view<decltype(ob),char32_t> view(ob);
	std::string_view const emoji(u8"☭🚄😀我wubi😄😊😉😍😘😚😜😝😳😁😣😢😂😭😪😥😰😩㍿🀀🀁🀂🀃🀄🀅🀆🀇🀈🀉🀊🀋🀌🀍🀎🀏");
	println(ob,"original emoji string:\n",emoji);
	for(auto const & e : emoji)
		print(ob,unsigned_view(e)," ");
	print(ob,"\n\n");
	auto u32emoji(utf8_to_unicode<std::u32string>(emoji));
	println(ob,"unicode characters: ",u32emoji.size());
	for(auto const & e : u32emoji)
	{
		view.put(e);
		fprint(ob," % (0x%)\n",unsigned_view(e),hexupper(unsigned_view(e)));
	}
	println(ob);
	auto const convert_back(unicode_to_utf8(u32emoji));
	println(ob,"convert back emoji string:\n",emoji);
	for(auto const& e : convert_back)
		print(ob,unsigned_view(e)," ");
	println(ob,"\n\nSame?\n",emoji==convert_back);
}