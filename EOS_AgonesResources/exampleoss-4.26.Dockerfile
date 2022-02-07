FROM ubuntu:20.04

RUN useradd -m ue4
USER ue4:ue4

EXPOSE 7777/udp
EXPOSE 12345/udp

ADD --chown=ue4:ue4 ExampleCPP_EOS_4.26/OutputServer/LinuxServer /home/ue4

ENTRYPOINT [ "/home/ue4/ExampleCPP_EOS_4.26Server.sh" ]