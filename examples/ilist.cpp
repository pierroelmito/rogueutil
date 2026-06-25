
#include "rupp.hpp"

#include <optional>
#include <string>
#include <vector>

struct Location {
	size_t start { };
	size_t selected { };
	size_t previousSel { };
};

struct Data {
	std::string line { };
	std::string actionContent { };
	std::string actionCommand { };
	std::string currentCommand { };
	std::vector<std::string> content;
	Location loc { };
	int frame { };
	int action { };
	static Data& get()
	{
		static Data v { };
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

void Draw(rupp::Vec d, size_t frameIndex)
{
	auto& data = Data::get();

	data.line = std::string(d.x, ' ');
	data.frame += 1;
	auto eol = [&](int sz) { rupp::fmt("%s", data.line.data() + sz); };

	bool fullRedraw = frameIndex == 0;

	const auto s = data.loc.selected;
	if (s < data.loc.start) {
		fullRedraw = true;
		data.loc.start = s;
	} else if (s > data.loc.start + d.y - 2) {
		fullRedraw = true;
		data.loc.start = s - d.y + 2;
	}

	if (fullRedraw)
		rupp::put(rupp::Cls);

	// status bar
	{
		put(rupp::Vec { 1, d.y }, rupp::Bg { 238 });
		rupp::put(rupp::Bg { 70 }, rupp::Fg { 255 });
		const int sz = rupp::fmt("[%ld/%ld] - %s", 1 + data.loc.selected, data.content.size(), data.currentCommand.c_str());
		eol(sz);
	}

	// table
	{
		put(rupp::Reset);
		for (int y = 0; y < d.y - 1; ++y) {
			const size_t index = y + data.loc.start;
			if (!fullRedraw) {
				if (index != s && index != data.loc.previousSel)
					continue;
			}
			put(rupp::Vec { 1, 1 + y });
			if (index < data.content.size()) {
				const auto& line = data.content[index];
				if (index == data.loc.selected) {
					rupp::fmt(rupp::Reset, rupp::Bg { 26 }, rupp::Fg { 227 });
					const int sz = rupp::fmt("%s", line.c_str());
					eol(sz);
					put(rupp::Reset);
				} else {
					const int sz = rupp::fmt(rupp::Fg { 254 }, "%s", line.c_str());
					if (!fullRedraw)
						eol(sz);
				}
			} else {
				break;
			}
		}
	}

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

std::string Replace(std::string str, const std::string& orig, const std::string& rep)
{
	size_t pos = str.find(orig);
	while (pos != std::string::npos) {
		str.replace(pos, orig.size(), rep);
		pos = str.find(orig, pos + rep.size());
	}
	return str;
}

bool Input(rupp::Vec d, int k)
{
	if (k == 'q')
		return false;
	auto& data = Data::get();
	size_t& s = data.loc.selected;
	if (auto npos = scroll(s, data.content.size(), d.y - 2, k); npos) {
		data.loc.previousSel = s;
		s = *npos;
		data.currentCommand = Replace(data.actionCommand, "%l", data.content[s]);
	} else if (k == rupp::KeyCode::Enter) {
		data.action += 1;
		if (!data.content.empty()) {
			const std::string& cmd = data.currentCommand;
			rupp::setCursor(true);
			system(cmd.c_str());
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
	if (data.content.empty())
		return 1;
	data.currentCommand = Replace(data.actionCommand, "%l", data.content[data.loc.selected]);
	rupp::tui(Draw, Input);
	return 0;
}
