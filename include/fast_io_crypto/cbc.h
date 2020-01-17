#pragma once

namespace fast_io::crypto
{

template<typename ciphert,bool Enc>
class cbc
{
public:
	using cipher_type = ciphert;
	using key_type = std::span<std::byte const, cipher_type::key_size>;
	using block_type = std::span<std::byte, cipher_type::block_size>;
	using iv_type = std::array<std::byte, cipher_type::block_size>;
	inline static constexpr std::size_t block_size = cipher_type::block_size;

public:
	iv_type iv;
	cipher_type cipher;
	[[deprecated("CBC is no longer secure due to a padding oracle attack. https://en.wikipedia.org/wiki/Padding_oracle_attack#Padding_oracle_attack_on_CBC_encryption")]] cbc(key_type key, block_type iv2):cipher(key)
	{
		details::my_copy_n(iv2.begin(), cipher_type::block_size, iv.data());
	}
	auto operator()(std::span<std::byte, block_size> plain_cipher_text)
	{
		if constexpr(Enc)
		{
			for (std::size_t i{}; i != iv.size(); ++i)
				plain_cipher_text[i] ^= iv[i];
			auto cipher_text(cipher(plain_cipher_text.data()));
			details::my_copy(cipher_text.begin(), cipher_text.end(), iv.data());
			return cipher_text;
		}
		else if constexpr(!Enc)
		{
			auto plain(cipher(plain_cipher_text.data()));
			for (std::size_t i{}; i != iv.size(); ++i)
				plain[i] ^= iv[i];
			details::my_copy(plain_cipher_text.begin(), plain_cipher_text.end(), iv.data());
			return plain;
		}
	}
	auto digest(std::span<std::byte const> inp) requires (Enc)
	{
		std::array<std::byte, block_size> plain_text{};
		details::my_copy(inp.begin(), inp.end(), plain_text.data());
		for (std::size_t i{}; i != iv.size(); ++i)
			plain_text[i] ^= iv[i];
		auto const cipher_text(cipher(plain_text.data()));
		details::my_copy(cipher_text.begin(), cipher_text.end(), iv.data());
		return cipher_text;
	}
};

template<buffer_output_stream T, typename Enc, std::size_t sz = 4096>
using ocbc_encrypt = otransform<T, block_processor<cbc<Enc,true>>, typename T::char_type, sz>;

template<buffer_input_stream T, typename Dec, std::size_t sz = 4096>
using icbc_decrypt = itransform<T, block_processor<cbc<Dec,false>>, typename T::char_type, sz>;

template<buffer_output_stream T, typename Dec, std::size_t sz = 4096>
using ocbc_decrypt = otransform<T, block_processor<cbc<Dec,false>>, typename T::char_type, sz>;

template<buffer_input_stream T, typename Enc, std::size_t sz = 4096>
using icbc_encrypt = itransform<T, block_processor<cbc<Enc,true>>, typename T::char_type, sz>;

}
