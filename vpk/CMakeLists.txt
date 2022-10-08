cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if(DEFINED ENV{VITASDK})
		set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
	else()
		message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
	endif()
endif()

set(SHORT_NAME vitapad)
project(${SHORT_NAME})
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "VitaPad")
set(VITA_TITLEID  "VITAPAD00")
set(VITA_VERSION "01.04")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format-truncation -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2808000000000000")

if (NOT ${RELEASE})
	add_definitions(-DENABLE_LOGGING)
endif()

FUNCTION(ADD_RESOURCES out_var)
	SET(result)
	FOREACH(ref_f ${ARGN})
		if (IS_ABSOLUTE "${ref_f}")
			SET(out_f "${ref_f}.o")
			STRING(REPLACE "${CMAKE_CURRENT_BINARY_DIR}/" "" in_f "${ref_f}")
			SET(work_dir "${CMAKE_CURRENT_BINARY_DIR}")
		else()
			SET(out_f "${CMAKE_CURRENT_BINARY_DIR}/${ref_f}.o")
			SET(in_f "${ref_f}")
			SET(work_dir "${CMAKE_SOURCE_DIR}")
		endif()
		GET_FILENAME_COMPONENT(out_dir ${out_f} DIRECTORY)
		ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
				COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
				COMMAND ${CMAKE_LINKER} -r -b binary -o ${out_f} ${in_f}
				DEPENDS ${ref_f}
				WORKING_DIRECTORY ${work_dir}
				COMMENT "Building resource ${out_f}"
				VERBATIM
				)
		LIST(APPEND result ${out_f})
	ENDFOREACH()
	SET(${out_var} "${result}" PARENT_SCOPE)
ENDFUNCTION()

include_directories(
	../skprx/uapi
)

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	../skprx/stubs
)

file(GLOB res_files RELATIVE
	${CMAKE_SOURCE_DIR}
	resources/images/*.png
	resources/default/*.txt
	resources/*.txt
)

add_resources(vitapad_res
	${res_files}
)

add_executable(${SHORT_NAME}
	${vitapad_res}
	main.c
	touchpad.c
	utils.c
	font.c
	modal.c
	menu.c
	settings.c
)

target_link_libraries(${SHORT_NAME}
	vitapad_stub
	vita2d
	png
	jpeg
	z
	m
	c
	crypto
	taihen_stub
	ScePgf_stub
	SceDisplay_stub
	SceTouch_stub
	SceCtrl_stub
	ScePower_stub
	SceMtpIfDriver_stub
	SceUsbstorVStorDriver_stub
	SceAppMgr_stub
	ScePower_stub
	SceShellSvc_stub
	SceGxm_stub
	SceSysmodule_stub
	SceCommonDialog_stub
	SceAVConfig_stub
)

vita_create_self(${SHORT_NAME}.self ${SHORT_NAME})

vita_create_vpk(${SHORT_NAME}.vpk ${VITA_TITLEID} ${SHORT_NAME}.self
	VERSION ${VITA_VERSION}
	NAME ${VITA_APP_NAME}
	FILE	pkg/sce_sys/icon0.png sce_sys/icon0.png
		pkg/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
		pkg/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
		pkg/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)

add_custom_target(vpksend
	COMMAND curl -T ${SHORT_NAME}.vpk ftp://$(PSVITAIP):1337/ux0:/
	DEPENDS ${SHORT_NAME}.vpk
)

add_custom_target(send
	COMMAND curl -T ${SHORT_NAME}.self ftp://$(PSVITAIP):1337/ux0:/app/${VITA_TITLEID}/eboot.bin
	DEPENDS ${SHORT_NAME}.self
)
