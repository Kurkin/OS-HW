BASE=cat
C=$(BASE).c
SHELL=bash -O extglob

all: $(BASE)

clear: 
	rm -f !($(C)|Makefile)

%: %.c
	cc $< -o $@ 

