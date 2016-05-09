add_definitions(-DCGAMEDLL)

add_library(cgame SHARED
	../game/bg_animation.c
	../game/bg_misc.c
	../game/bg_pmove.c
	../game/bg_slidemove.c
	../game/q_math.c
	../game/q_shared.c
	../ui/ui_shared.c
	cg_consolecmds.c
	cg_draw.c
	cg_drawtools.c
	cg_effects.c
	cg_ents.c
	cg_event.c
	cg_flamethrower.c
	cg_info.c
	cg_localents.c
	cg_main.c
	cg_marks.c
	cg_newDraw.c
	cg_particles.c
	cg_players.c
	cg_playerstate.c
	cg_predict.c
	cg_scoreboard.c
	cg_servercmds.c
	cg_snapshot.c
	cg_sound.c
	cg_syscalls.c
	cg_trails.c
	cg_view.c
	cg_weapons.c
)
target_link_libraries(cgame m)
set_target_properties(cgame PROPERTIES PREFIX "" OUTPUT_NAME "cgame-rtcw")
