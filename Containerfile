FROM ruby:2.7.1-buster

RUN apt-get update && apt-get install -y gcc-arm-none-eabi gdb-multiarch minicom && rm -rf /var/lib/apt/lists/*
RUN useradd --create-home stm32dev

ENV LANG=C.UTF-8

USER stm32dev

RUN mkdir /home/stm32dev/src
WORKDIR /home/stm32dev/src
COPY Gemfile* .

RUN bundle install

LABEL SHELL="podman run \
  --rm \
  -it \
  -v .:/home/stm32dev/src:Z \
  -v /dev/ttyACM0:/dev/ttyACM0 \
  -v /dev/ttyACM1:/dev/ttyACM1 \
  --name stm32dev \
  --userns=keep-id \
  --security-opt label=type:stm32dev_container.process \
  IMAGE \
  bash"
