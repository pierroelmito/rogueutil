
#include "rupp.hpp"

#include <array>

template <typename T>
void example(const char* label, unsigned char i)
{
	using namespace rupp;
	const auto green = rgb(0, i, 0);
	const auto red = rgb(i, 0, 0);
	const auto blue = rgb(0, 0, i);
	const std::array fgs { T { green }, T { blue }, T { red } };
	put(label, " examples: ", fgs[0], "green", Reset, ", ", fgs[1], "blue", Reset, ", ", fgs[2], "red", Reset, "...\n");
}

int main(int, char**)
{
	using namespace rupp;
	put("\n");
	example<Fg>("fg", 5);
	put("\n");
	example<Bg>("bg", 1);
	put("\n");
	for (unsigned char i = 0, idx = 0; i < 16; ++i) {
		for (unsigned char j = 0; j < 16; ++j, ++idx)
			fmt(Fg { idx }, " %3d ", idx);
		put("\n");
	}
	put("\n");
	for (unsigned char i = 0, idx = 0; i < 16; ++i) {
		for (unsigned char j = 0; j < 16; ++j, ++idx)
			fmt(Bg { idx }, " %3d ", idx);
		put("\n");
	}
	put("\n");
	return 0;
}
