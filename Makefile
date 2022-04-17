all: slave master
	

slave:
	gcc -o slave slave.c -pthread -lrt

master:
	gcc -o master master.c -pthread -lrt
	
clean:
	rm logfile*
	rm cstest
	rm master
	rm slave
