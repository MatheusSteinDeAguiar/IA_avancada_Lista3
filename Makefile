main :
	clear
	ulimit -t 60; 
	ulimit -v 2000000 
	./fast-downward/fast-downward.py --test-and-or-graphs

