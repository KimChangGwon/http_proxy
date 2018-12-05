all: http_proxy

http_proxy: http_proxy.o
	gcc -g -o http_proxy http_proxy.o -lpthread

http_proxy.o:
	gcc -g -c -o http_proxy.o http_proxy.c

clean:
	rm -f http_proxy
	rm -f *.o
