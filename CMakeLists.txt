CMAKE_MINIMUM_REQUIRED(VERSION 3.15..4.3.2)
SET(CMAKE_TOOLCHAIN_FILE "toolchain.cmake")
PROJECT(lethe VERSION 0.1 DESCRIPTION "Operating system targeting x86_64 platforms" LANGUAGES C)
SET(CMAKE_INSTALL_PREFIX "")
UNSET(CMAKE_INSTALL_PREFIX)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

ADD_COMPILE_OPTIONS(-ffreestanding
        -nostdlib
        -fshort-wchar
        -mno-red-zone
        -fno-stack-protector
        -mno-mmx
        -mno-sse
        -mno-sse2
        -fno-PIC
        -fno-PIE
        $<$<CONFIG:DEBUG>:-g>
        $<$<NOT:$<CONFIG:DEBUG>>:-O2>)

ADD_LINK_OPTIONS(
        -nostdlib
        --static
        -Wl,-z,max-page-size=0x1000
        $<$<NOT:$<CONFIG:DEBUG>>:-O2>)
ADD_COMPILE_DEFINITIONS(__is_libk)
INCLUDE_DIRECTORIES(lethe-kernel/include)
INCLUDE_DIRECTORIES(lethe-libc/include)
ADD_SUBDIRECTORY(lethe-libc)
ADD_COMPILE_OPTIONS(-mcmodel=large)
#SET(ENV{LETHE_BOUNCER} 1)
#IF (DEFINED ENV{FEATURE_QEMU})
	SET(FEATURE_QEMU 1)
	#ENDIF()
	#IF (DEFINED ENV{FEATURE_LIMINE})
    SET(FEATURE_LIMINE 1)
    #ELSEIF (DEFINED ENV{FEATURE_EFI_LOADER})
    #ELSE ()
    #SET(FEATURE_LIMINE 1)
    #ENDIF ()
file(CREATE_LINK
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  SYMBOLIC
)
#ADD_SUBDIRECTORY(lethe-aml)
ADD_SUBDIRECTORY(lethe-aml/third_party)
ADD_SUBDIRECTORY(lethe-aml/aml)
TARGET_INCLUDE_DIRECTORIES(fnv PRIVATE lethe-aml/aml/include)
ADD_SUBDIRECTORY(lethe-kernel)
ADD_SUBDIRECTORY(lethe-bootloaders)
TARGET_INCLUDE_DIRECTORIES(acpi PRIVATE lethe-aml/aml/include)
ADD_SUBDIRECTORY(templates)
