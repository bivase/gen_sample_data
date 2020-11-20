#ifndef BASE_H
#define BASE_H

#include "headers.h"

constexpr int ANY_NUMBER{100};
constexpr int NUM_OF_SKIP{2};

namespace
{
	struct file
			: std::unique_ptr<FILE, decltype(&fclose)>
	{};
}

// GET BLOCK OFFSETS
inline std::vector<std::pair<size_t, size_t>>
get_block_offsets(std::string_view filename)
{
	namespace fs = std::filesystem;
	std::vector<std::pair<size_t, size_t>> blocks{};
	file src{{fopen(filename.data(), "r"), &fclose}};
	if (src.get() == nullptr)
	{
		std::cout << "Ошибка чтения файла: " << filename << std::endl;
		return std::vector<std::pair<size_t, size_t>>{};
	}
	size_t sz_file{static_cast<size_t>(fs::file_size(filename))};

	size_t kernels{std::thread::hardware_concurrency()};
	std::pair<size_t, size_t> block{};
	size_t sz_block{sz_file / kernels};
	for (size_t cur_pos{}; cur_pos < sz_file;)
	{
		block.first = cur_pos;
		std::array<char, ANY_NUMBER> buf{};
		size_t next_pos{cur_pos + sz_block};

		fseek(src.get(), next_pos, SEEK_SET);
		size_t was_read{fread(buf.data(), 1, buf.size(), src.get())};
		cur_pos = block.second = next_pos + 1 + std::distance(
									 buf.cbegin(),
									 std::find(
										 buf.cbegin(), std::next(buf.cbegin(), was_read),
										 '\n')
									 );
		blocks.emplace_back(block);
	}
	return blocks;
}

// GET INFO ACTIVITY
inline void get_info_activity(file& src, size_t offset_begin, size_t offset_end, std::string_view str, file& dst)
{
	std::vector<uint8_t> buf(offset_end - offset_begin);
	fseek(src.get(), offset_begin, SEEK_SET);
	fread(buf.data(), 1, buf.size(), src.get());

	auto iter{std::search(
					buf.cbegin(), buf.cend(),
					str.cbegin(), str.cend()
					)
			 };

	while(iter != buf.cend())
	{
		auto end{std::make_reverse_iterator(iter)};
		auto begin{std::next(
						std::find(
							end, std::make_reverse_iterator(buf.cbegin()),
							'+'
							)
						, 1)
				  };
		std::string out{std::string{std::make_reverse_iterator(begin),std::make_reverse_iterator(end)} + "\n"};
		fwrite(out.data(), 1, out.size(), dst.get());
		iter = std::search(
				   std::next(iter, 1), buf.cend(),
				   str.cbegin(), str.cend()
				   );
	}
}
// MULTI GET INFO ACTIVITY
inline void multi_get_info_activity(std::string_view filename, std::string_view str, std::string_view filename_out)
{
	file src{{fopen(filename.data(), "r"), &fclose}};
	if (src.get() == nullptr)
	{
		std::cout << "Ошибка чтения файла: " << filename << std::endl;
		return;
	}
	std::vector<std::thread> threads{};
	auto offsets{get_block_offsets(filename)};
	file dst{{fopen(filename_out.data(), "w+"), &fclose}};
	for (auto&& offset : offsets)
	{
		threads.emplace_back(get_info_activity, std::ref(src), offset.first, offset.second, str, std::ref(dst));
	}
	for (auto&& thread : threads)
	{
		thread.join();
	}
}
// GET INFO PHONE
inline void get_info_phone(file& src, size_t offset_begin, size_t offset_end, std::vector<std::string_view>& list_strs, file& dst)
{
	std::vector<uint8_t> buf(offset_end - offset_begin);
	fread(buf.data(), 1, buf.size(), src.get());

	for (auto&& str : list_strs)
	{
		auto iter{std::search(
						buf.cbegin(), buf.cend(),
						str.cbegin(), str.cend()
						)
				 };
		while(iter != buf.cend())
		{
			auto begin{std::next(iter, str.size() + NUM_OF_SKIP)};
			auto end{std::find(
							begin, std::next(begin, offset_end),
							'\n')
						 };
			std::string out{std::string{begin, end} + "\n"};
			fwrite(out.data(), 1, out.size(), dst.get());
			iter = std::search(
					   end, buf.cend(),
					   str.cbegin(), str.cend()
					   );
		}
	}
}
// MULTI GET INFO PHONE
inline void multi_get_info_phone(std::string_view filename, std::vector<std::string_view>& lst, std::string_view filename_out)
{
	file src{{fopen(filename.data(), "r"), &fclose}};
	if (src.get() == nullptr)
	{
		std::cout << "Ошибка чтения файла: " << filename << std::endl;
		return;
	}
	std::vector<std::thread> threads{};
	auto offsets{get_block_offsets(filename)};
	file dst{{fopen(filename_out.data(), "w+"), &fclose}};
	for (auto&& offset : offsets)
	{
		threads.emplace_back(get_info_phone, std::ref(src), offset.first, offset.second, std::ref(lst), std::ref(dst));
	}
	for (auto&& thread : threads)
	{
		thread.join();
	}
}

#endif // BASE_H
