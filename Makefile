###################################################
#
# file: Makefile
#
# @Author:   Ioannis Chatziantwoiou
# @Version:  6-11-2024
# @email:    csd5193@csd.uoc.gr
#
# Makefile
#
####################################################

all : main


main : main.o
	gcc -pthread main.o -o main

main.o : main.c
	gcc -pthread -c main.c

clean :
	rm -f *.o
	rm -f main

