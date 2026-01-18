
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
end

addExample("rucolors")
addExample("ilist")

