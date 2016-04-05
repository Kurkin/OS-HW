BASE=midtermsh
C=$(BASE).c

all: $(BASE)

clean: 
	    rm -f $(BASE)

%: %.c
	    clang++ $< -o $@ -std=c++11
