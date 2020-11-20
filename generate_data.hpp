#ifndef BASE_H
#define BASE_H

#include "headers.h"

constexpr int MAX_NUM_PHONE{10};
constexpr char ZERO{0x30};
constexpr char NINE{0x39};

namespace
{
	struct file
			: std::unique_ptr<FILE, decltype(&fclose)>
	{};
}

// RANDOM DATA
template <int SIZE>
class rnd
		: public std::array<std::string_view, SIZE>
{
	static inline std::random_device gen{};
	static inline std::uniform_int_distribution dist{0, SIZE - 1};
public:
	using rnd_t = std::array<std::string_view, SIZE>;
	const std::string_view generate() const noexcept; // GENERATE
};
template <int SIZE>
using rnd_t = typename rnd<SIZE>::rnd_t;
// GENERATE
template <int SIZE>
const std::string_view rnd<SIZE>::generate() const noexcept
{
	return (*this)[dist(gen)];
}

// DATABASE
rnd_t<14> db_names
{
	"Иван",
	"Пётр",
	"Харитон",
	"Радион",
	"Артём",
	"Илья",
	"Михаил",
	"Алексей",
	"Сергей",
	"Антон",
	"Митяй",
	"Виктор",
	"Виталий",
	"Григорий"
};
rnd_t<10> db_surnames
{
	"Петров",
	"Иванов",
	"Сидоров",
	"Хазанов",
	"Мартиросян",
	"Матросов",
	"Григорьев",
	"Васильев",
	"Жаров",
	"Меркулов"
};
rnd_t<10> db_patronymics
{
	"Сергеевич",
	"Иванович",
	"Петрович",
	"Радионович",
	"Джибраилович",
	"Алексеевич",
	"Юрьевич",
	"Дмитриевич",
	"Константинович",
	"Михайлович"
};
// GENERATE PHONE
inline auto generate_phone(std::string_view prefix)
{
	return [prefix] ()
	{
		static std::random_device gen{};
		static std::uniform_int_distribution dist{ZERO, NINE};
		std::string out{prefix};
		for (size_t i{MAX_NUM_PHONE}; i; --i)
		{
			out += dist(gen);
		}
		return out;
	};
}
// GENERATE FIO
inline auto generate_fio()
{
	return []
	{
		rnd<db_names.size()> names{db_names};
		rnd<db_surnames.size()> surnames{db_surnames};
		rnd<db_patronymics.size()> patronymics{db_patronymics};
		return std::string{
			static_cast<std::string>(surnames.generate()) + " "
					+ static_cast<std::string>(names.generate()) + " "
					+ static_cast<std::string>(patronymics.generate())
		};
	};
}
// GENERATE ACTIVITY SIGN
inline auto generate_activity_sign(std::string_view on, std::string_view off)
{
	return [on, off]
	{
		rnd<2> activity_sign{{on, off}};
		return static_cast<std::string>(activity_sign.generate());
	};
}
// MULTI GENERATE DATA
template <class... Args>
inline void multi_generate_data(std::string_view filename, size_t iterations, Args... args)
{
	file dst{{fopen(filename.data(), "a"), &fclose}};
	auto generate_data
	{
		[&dst, ... args{args}] (size_t size) mutable
		{
			while (--size)
			{
				std::string out{(("\"" + args() + "\",") + ... + "\n")};
				fwrite(out.data(), 1, out.size(), dst.get());
			}
		}
	};
	std::vector<std::thread> threads{};
	for (uint32_t i{std::thread::hardware_concurrency()}; i; --i)
	{
		threads.emplace_back(generate_data, iterations);
	}
	for (auto&& thread : threads)
	{
		thread.join();
	}
}

#endif // BASE_H
