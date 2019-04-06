add_definitions(-DCGAMEDLL)

add_library(cgame.a STATIC
	../game/bg_animation.c
	../game/bg_misc.c
	../game/bg_pmove.c
	../game/bg_slidemove.c
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
	cg_static.c
	cg_trails.c
	cg_view.c
	cg_weapons.c
)
target_link_libraries(cgame.a m)
set_target_properties(cgame.a PROPERTIES PREFIX "" SUFFIX "")
