
add_rules("mode.debug", "mode.release")

function addExample(name)
	target(name)
	set_policy("build.warning", true)
	set_warnings("all", "extra")
	set_kind("binary")
	add_files("examples/" .. name .. ".cpp")
	add_cxflags("-std=c++20")
	add_includedirs(".")
	set_rundir(".")
	if is_mode("debug") then
		add_defines("DEBUG")
	end
	if is_os("windows") then
		add_defines("USE_WIN_API")
		add_cxflags("-mwindows")
	end
end

addExample("rucolors")
addExample("ilist")

