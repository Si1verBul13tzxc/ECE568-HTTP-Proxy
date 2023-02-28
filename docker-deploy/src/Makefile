CC = g++
CXXFLAGS=-std=c++11 -pedantic -Wall -Werror -ggdb3 -pthread
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
proxy:$(OBJS)
	g++ $(CXXFLAGS) -o $@ $(OBJS)
.PHONY: clean depend
clean:
	rm -f proxy *.o *~
depend:
	makedepend $(SRCS)
# DO NOT DELETE

parser_method.o: parser_method.hpp httpparser/httprequestparser.h
parser_method.o: /usr/include/string.h /usr/include/stdlib.h
parser_method.o: httpparser/request.h httpparser/httpresponseparser.h
parser_method.o: httpparser/response.h httpparser/request.h
parser_method.o: httpparser/response.h my_exception.hpp
proxy.o: proxy.hpp /usr/include/assert.h /usr/include/features.h
proxy.o: /usr/include/stdc-predef.h /usr/include/poll.h Socket_connection.hpp
proxy.o: /usr/include/arpa/inet.h /usr/include/netinet/in.h
proxy.o: /usr/include/endian.h /usr/include/netdb.h /usr/include/rpc/netdb.h
proxy.o: /usr/include/unistd.h my_exception.hpp parser_method.hpp
proxy.o: httpparser/httprequestparser.h /usr/include/string.h
proxy.o: /usr/include/stdlib.h httpparser/request.h
proxy.o: httpparser/httpresponseparser.h httpparser/response.h
proxy.o: httpparser/request.h httpparser/response.h cache.hpp
Socket_connection.o: Socket_connection.hpp /usr/include/arpa/inet.h
Socket_connection.o: /usr/include/features.h /usr/include/stdc-predef.h
Socket_connection.o: /usr/include/netinet/in.h /usr/include/endian.h
Socket_connection.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
Socket_connection.o: /usr/include/unistd.h my_exception.hpp
main.o: proxy.hpp /usr/include/assert.h /usr/include/features.h
main.o: /usr/include/stdc-predef.h /usr/include/poll.h Socket_connection.hpp
main.o: /usr/include/arpa/inet.h /usr/include/netinet/in.h
main.o: /usr/include/endian.h /usr/include/netdb.h /usr/include/rpc/netdb.h
main.o: /usr/include/unistd.h my_exception.hpp parser_method.hpp
main.o: httpparser/httprequestparser.h /usr/include/string.h
main.o: /usr/include/stdlib.h httpparser/request.h
main.o: httpparser/httpresponseparser.h httpparser/response.h
main.o: httpparser/request.h httpparser/response.h cache.hpp
cache.o: cache.hpp /usr/include/assert.h /usr/include/features.h
cache.o: /usr/include/stdc-predef.h /usr/include/poll.h Socket_connection.hpp
cache.o: /usr/include/arpa/inet.h /usr/include/netinet/in.h
cache.o: /usr/include/endian.h /usr/include/netdb.h /usr/include/rpc/netdb.h
cache.o: /usr/include/unistd.h my_exception.hpp parser_method.hpp
cache.o: httpparser/httprequestparser.h /usr/include/string.h
cache.o: /usr/include/stdlib.h httpparser/request.h
cache.o: httpparser/httpresponseparser.h httpparser/response.h
cache.o: httpparser/request.h httpparser/response.h
