# DO NOT USE `RUN.SH` OR `APPLY.SH` UNLESS YOU'RE CERTAIN THAT NOTHING IMPORTANT IS MOUNTED ON /DEV/SDA1
apply.sh assumes that a flash drive is mounted on /dev/sda1, if you have a hard disk or a flash drive that you'd like to keep the data on, DO NOT USE IT or at least change the disk path to something else

# Using modified ELF loader from [krinkinmu/efi](https://github.com/krinkinmu/efi/tree/master)
Specifically added `BootServices.LocateProtocol` and `EFI_GRAPHICS_OUTPUT_PROTOCOL` support
License for the loader can be found at `loader/LICENSE`
