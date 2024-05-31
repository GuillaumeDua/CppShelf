ARG BASE_IMAGE=ubuntu:latest
FROM ${BASE_IMAGE} as ssh-support

RUN apt-get update && apt-get install -qqy \
    openssh-server  \
    rsync
RUN mkdir /var/run/sshd

# WIP: no password required
#   https://dev.to/emmanuelnk/using-sudo-without-password-prompt-as-non-root-docker-user-52bg
RUN echo 'root:' | chpasswd
# passwd -d root
# echo 'root::0:0:root:/root:/bin/bash' > /etc/passwd
RUN sed -i 's/PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config \
    && sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config \
    && sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
