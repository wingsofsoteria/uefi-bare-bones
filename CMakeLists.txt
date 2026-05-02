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
INCLUDE_DIRECTORIES(lethe-libc/include)
ADD_SUBDIRECTORY(lethe-libc)
ADD_COMPILE_OPTIONS(-mcmodel=large)
#SET(ENV{LETHE_BOUNCER} 1)
IF (DEFINED ENV{FEATURE_QEMU})
	SET(FEATURE_QEMU 1)
ENDIF()
IF (DEFINED ENV{FEATURE_LIMINE})
    SET(FEATURE_LIMINE 1)
ELSEIF (DEFINED ENV{FEATURE_EFI_LOADER})
ELSE ()
    SET(FEATURE_LIMINE 1)
ENDIF ()
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
SET(REL_INSTALL_DIR "lethe-misc/out")
SET(REL_SCRIPT_DIR "lethe-misc")
CMAKE_PATH(ABSOLUTE_PATH REL_INSTALL_DIR OUTPUT_VARIABLE ABS_INSTALL_DIR)
CMAKE_PATH(ABSOLUTE_PATH REL_SCRIPT_DIR OUTPUT_VARIABLE ABS_SCRIPT_PATH)
SET(LETHE_TEMP_DIR ${PROJECT_BINARY_DIR}/tmp)
SET(LETHE_OUTPUT_FILE ${ABS_SCRIPT_PATH}/fat.img)
IF (DEFINED ENV{FEATURE_LIMINE})
    SET(REL_BOOTLOADER_PATH "lethe-bootloaders")
    CMAKE_PATH(ABSOLUTE_PATH REL_BOOTLOADER_PATH OUTPUT_VARIABLE ABS_BOOTLOADER_PATH)
    SET(LETHE_BOOTLOADER_CUSTOM_FILES "${ABS_BOOTLOADER_PATH}/limine.conf" "${ABS_BOOTLOADER_PATH}/limine/limine-bios.sys")
    SET(LETHE_BOOTLOADER_FILES "${ABS_BOOTLOADER_PATH}/limine/BOOTX64.EFI" "${ABS_BOOTLOADER_PATH}/limine/BOOTIA32.EFI")
    SET(LETHE_BOOTLOADER_CUSTOM_DESTINATION "boot/limine")
    SET(LETHE_KERNEL_DESTINATION "boot")
    SET(LETHE_BOOTLOADER_DESTINATION "efi/boot")
ENDIF ()
CONFIGURE_FILE(image-template.sh ${PROJECT_BINARY_DIR}/build-image.sh)
CONFIGURE_FILE(apply-template.sh ${PROJECT_BINARY_DIR}/apply.sh)
SET(LETHE_TEST_FW_VAR ${ABS_SCRIPT_PATH}/bin/OVMF_VARS.fd)
SET(LETHE_TEST_FW_CODE ${ABS_SCRIPT_PATH}/bin/OVMF_CODE.fd)
file(WRITE ${PROJECT_BINARY_DIR}/run.sh
        "#!/usr/bin/env bash\n"
        "qemu-system-x86_64 "
        "-net none "
        "-no-reboot "
        "-enable-kvm "
        "-cpu host,+invtsc "
        "-m 48G "
        "-drive if=pflash,format=raw,readonly=on,unit=0,file=${LETHE_TEST_FW_CODE} "
        "-drive if=pflash,format=raw,readonly=on,unit=1,file=${LETHE_TEST_FW_VAR} "
        "-drive file=${LETHE_OUTPUT_FILE} ")
IF (DEFINED ENV{FEATURE_QEMU})
    FILE(APPEND ${PROJECT_BINARY_DIR}/run.sh "-debugcon stdio 1> >(tee log >&2)")
ELSE ()
    FILE(APPEND ${PROJECT_BINARY_DIR}/run.sh "-monitor stdio 1> >(tee log >&2)")
ENDIF ()
FILE(CHMOD ${PROJECT_BINARY_DIR}/run.sh FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ADD_CUSTOM_TARGET(images ${PROJECT_BINARY_DIR}/build-image.sh)
ADD_CUSTOM_TARGET(qemu ${PROJECT_BINARY_DIR}/run.sh)
