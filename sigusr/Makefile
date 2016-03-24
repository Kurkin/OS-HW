BASE=sigusr
C=$(BASE).c

all: $(BASE)

clean: 
	rm -f $(BASE) 

%: %.c
	cc $< -o $@ 

