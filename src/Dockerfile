FROM ubuntu:18.04
RUN mkdir /var/log/erss
RUN mkdir /src
WORKDIR /src
RUN apt-get update && apt-get -y install g++ make
ADD proxy.log /var/log/erss
ADD . /src