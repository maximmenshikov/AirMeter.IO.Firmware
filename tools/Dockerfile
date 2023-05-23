FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y cmake git libusb-1.0-0 libusb-1.0-0-dev ninja-build python3-pip python3-venv zip

RUN mkdir -p /.cache && chmod 777 /.cache
