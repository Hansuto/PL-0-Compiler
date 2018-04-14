# Program-3

COMPILATION COMMANDS
---------------------
gcc -o compile driver.c lexical.c parser.c vm.c

./compile filename.txt -l -a -v

---------------------

*UPDATE* 4.12.18 - diver.c is causing linking error due to multiple definitions. I will implement some header files to resolve this issue tomorrow. 

*UPDATE* 4.14.18 - lexical.h and vm.h have been created (with parcer.h coming too). If program is ran with only "-l" flag, it passes diff test with testsctipts. Not able to implement vm just yet. I'm not sure where to get input for it in the proper format.

input format for VM example:

01 00 00 05
01 01 00 03
06 00 00 06
04 00 00 04
04 01 00 05
09 00 00 01
09 01 00 01
09 00 00 03
