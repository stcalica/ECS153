all: runpriv

runpriv.o :  runpriv.cpp
		g++ -g -Wall runpriv.cpp -c

runpriv: runpriv.o
		g++ -g runpriv.o -o runpriv


clean:
		rm -rf runpriv.o runpriv
