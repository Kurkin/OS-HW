BASE=cat
C=$(BASE).c
SHELL=bash -O extglob

all: $(BASE)

clean: 
	rm -f cat

%: %.c
	cc $< -o $@ 

