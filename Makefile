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
proxy.o: /usr/include/netdb.h /usr/include/netinet/in.h /usr/include/endian.h
proxy.o: /usr/include/rpc/netdb.h /usr/include/string.h /usr/include/unistd.h
proxy.o: my_exception.hpp parser_method.hpp httpparser/httprequestparser.h
proxy.o: /usr/include/stdlib.h httpparser/request.h
proxy.o: httpparser/httpresponseparser.h httpparser/response.h
proxy.o: httpparser/request.h httpparser/response.h
Socket_connection.o: Socket_connection.hpp /usr/include/netdb.h
Socket_connection.o: /usr/include/features.h /usr/include/stdc-predef.h
Socket_connection.o: /usr/include/netinet/in.h /usr/include/endian.h
Socket_connection.o: /usr/include/rpc/netdb.h /usr/include/string.h
Socket_connection.o: /usr/include/unistd.h my_exception.hpp
main.o: proxy.hpp /usr/include/assert.h /usr/include/features.h
main.o: /usr/include/stdc-predef.h /usr/include/poll.h Socket_connection.hpp
main.o: /usr/include/netdb.h /usr/include/netinet/in.h /usr/include/endian.h
main.o: /usr/include/rpc/netdb.h /usr/include/string.h /usr/include/unistd.h
main.o: my_exception.hpp parser_method.hpp httpparser/httprequestparser.h
main.o: /usr/include/stdlib.h httpparser/request.h
main.o: httpparser/httpresponseparser.h httpparser/response.h
main.o: httpparser/request.h httpparser/response.h
