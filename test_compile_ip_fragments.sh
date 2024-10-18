set -eux

sh4-none-elf-as -g --isa=sh4 --little sg_ini.s -o sg_ini.o
sh4-none-elf-ld -T sg_ini.lds sg_ini.o -o sg_ini.elf
sh4-none-elf-objcopy -O binary sg_ini.elf sg_ini.bin
xxd sg_ini.bin sg_ini.bin.txt
sh4-none-elf-objcopy -O binary sg/sg_ini.obj sg/sg_ini.bin
xxd sg/sg_ini.bin sg/sg_ini.bin.txt

sh4-none-elf-as -g --isa=sh4 --little aip.s -o aip.o
sh4-none-elf-ld -T aip.lds aip.o -o aip.elf
sh4-none-elf-objcopy -O binary aip.elf aip.bin
xxd aip.bin aip.bin.txt
sh4-none-elf-objcopy -O binary sg/aip.obj sg/aip.bin
xxd sg/aip.bin sg/aip.bin.txt
