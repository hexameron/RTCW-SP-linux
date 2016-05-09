add_library(ui SHARED
	../game/bg_misc.c
	../game/q_math.c
	../game/q_shared.c
	ui_atoms.c
	ui_gameinfo.c
	ui_main.c
	ui_players.c
	ui_shared.c
	ui_syscalls.c
	ui_util.c
)
target_link_libraries(ui m)
set_target_properties(ui PROPERTIES PREFIX "" OUTPUT_NAME "ui-rtcw")
