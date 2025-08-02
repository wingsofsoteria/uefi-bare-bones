# DO NOT USE `RUN.SH` OR `APPLY.SH` UNLESS YOU'RE CERTAIN THAT NOTHING IMPORTANT IS MOUNTED ON /DEV/SDA1
apply.sh assumes that a flash drive is mounted on /dev/sda1, if you have a hard disk or a flash drive that you'd like to keep the data on, DO NOT USE IT or at least change the disk path to something else

# liballoc.c is from the [liballoc repository](https://github.com/blanham/liballoc/tree/master)
## while the implementation allows for thread safety, I have not implemented that and instead set the lock and unlock function to do nothing
