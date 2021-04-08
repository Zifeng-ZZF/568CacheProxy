CFLAGS=-pthread
CPPFLAGS=-std=c++11
HEADERS=HttpRequest.h HttpResponse.h
SRCS=entry.cpp Proxy.cpp SocketObject.cpp HttpParser.cpp LRUCache.cpp HttpRequest.cpp Logger.cpp
proxy: *.cpp *.h
	g++ $(CPPFLAGS) $(CFLAGS) -o $@ $(SRCS) $(HEADERS)
clean:
	rm -rf *.o *~