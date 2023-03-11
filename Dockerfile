FROM ubuntu

WORKDIR /code

COPY . .

ENV DEBIAN_FRONTEND=noninteractive

ENV TZ=America/Denver

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update

RUN apt install -y sudo

RUN sudo apt install -y  build-essential cmake git pkg-config libgtk-3-dev libjsoncpp-dev


RUN DEBIAN_FRONTEND=noninteractive sudo apt install -y libopencv-dev python3-opencv

RUN cd build && cmake .. && make

RUN sudo chmod +x /code/start.sh


