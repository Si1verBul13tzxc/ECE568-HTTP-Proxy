FROM gcc:latest
RUN mkdir /var/log/erss
add . /var/log/erss
RUN apt-get update && apt-get -y install g++ make
WORKDIR /var/log/erss
RUN chmod o+x ./proxy.sh

ENTRYPOINT ["./proxy.sh"]