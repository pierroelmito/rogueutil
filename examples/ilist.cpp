
#include "rupp.hpp"

#include <optional>
#include <string>
#include <vector>

struct Location {
	size_t start {};
	size_t selected {};
};

struct Data {
	std::string line {};
	std::string actionContent {};
	std::string actionCommand {};
	std::vector<std::string> content;
	Location loc {};
	int frame {};
	int action {};
	static Data& get()
	{
		static Data v {};
		return v;
	}
};

std::vector<std::string> getCommandOutput(const std::string& command)
{
	std::vector<std::string> outputLines;
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe) {
		return outputLines; // Return empty vector on failure
	}
	char buffer[256];
	while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
		auto& l = outputLines.emplace_back(buffer);
		if (l.back() == '\n')
			l.resize(l.size() - 1);
	}
	pclose(pipe);
	return outputLines;
}

void Draw(rupp::Vec d)
{
	auto& data = Data::get();

	data.line = std::string(d.x, ' ');
	data.frame += 1;
	auto eol = [&](int sz) { rupp::fmt("%s", data.line.data() + sz); };

	if (data.loc.selected < data.loc.start) {
		data.loc.start = data.loc.selected;
	} else if (data.loc.selected > data.loc.start + d.y - 2) {
		data.loc.start = data.loc.selected - d.y + 2;
	}

	rupp::put(rupp::Cls, rupp::Reset);

	// status bar
	{
		put(rupp::Vec { 1, d.y }, rupp::Bg { 238 });
		rupp::put(rupp::Bg { 70 }, rupp::Fg { 255 });
		const int sz = rupp::fmt("%d - %d - %d", data.frame, data.action, data.loc.selected);
		eol(sz);
	}

	// table
	{
		put(rupp::Reset);
		for (int y = 0; y < d.y - 1; ++y) {
			const size_t index = y + data.loc.start;
			if (index < data.content.size()) {
				put(rupp::Vec { 1, y + 1 });
				const auto& line = data.content[index];
				if (index == data.loc.selected) {
					rupp::fmt(rupp::Reset, rupp::Bg { 26 }, rupp::Fg { 227 });
					const int sz = rupp::fmt("%s", line.c_str());
					eol(sz);
					rupp::put(rupp::Reset, "\n");
				} else {
					rupp::fmt(rupp::Fg { 254 }, "%s\n", line.c_str());
				}
			} else {
				break;
			}
		}
	}

	rupp::put(rupp::Reset);
	fflush(stdout);
}

std::optional<size_t> scroll(size_t pos, size_t sz, size_t h, int k)
{
	if (sz == 0)
		return std::nullopt;
	const auto forward = [&](size_t delta) -> size_t {
		if (pos + delta < sz)
			return pos + delta;
		else
			return sz - 1;
	};
	const auto backward = [&](size_t delta) -> size_t {
		if (pos >= delta)
			return pos - delta;
		else
			return 0;
	};
	if (k == rupp::KeyCode::Up) {
		return backward(1);
	} else if (k == rupp::KeyCode::Down) {
		return forward(1);
	} else if (k == rupp::KeyCode::Left) {
		return backward(h);
	} else if (k == rupp::KeyCode::Right) {
		return forward(h);
	}
	return std::nullopt;
}

bool Input(rupp::Vec d, int k)
{
	if (k == 'q')
		return false;
	auto& data = Data::get();
	size_t& selected = data.loc.selected;
	if (auto npos = scroll(selected, data.content.size(), d.y - 2, k); npos) {
		selected = *npos;
	} else if (k == rupp::KeyCode::Enter) {
		data.action += 1;
		if (!data.content.empty()) {
			setenv("LINE", data.content[selected].c_str(), 1);
			rupp::setCursor(true);
			system(data.actionCommand.c_str());
			rupp::setCursor(false);
		}
	}
	return true;
}

int main(int ac, char** av)
{
	if (ac != 3)
		return 1;
	auto& data = Data::get();
	data.actionContent = av[1];
	data.actionCommand = av[2];
	data.content = getCommandOutput(data.actionContent);
	rupp::tui(Draw, Input);
	return 0;
}
