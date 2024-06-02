ARG BASE_IMAGE=ubuntu:latest
FROM ${BASE_IMAGE} as ssh-support

RUN apt-get update && apt-get install -qqy \
    openssh-server  \
    rsync
RUN mkdir /var/run/sshd

# Remote user (opt-in)
ARG USER_NAME
ARG USER_PASSWORD=default
RUN ([ -z "${USER_NAME}" ] || [ -z "${USER_PASSWORD}" ]) && "[ARG] USER_NAME and/or USER_PASSWORD is empty, no user will be created." \
    || (                                                        \
        echo "Adding user [${USER_NAME}] ..."                   \
        && useradd -m ${USER_NAME}                              \
        && echo "${USER_NAME}:${USER_PASSWORD}" | chpasswd      \
        # && adduser --disabled-password --gecos '' ${USER_NAME}   \
        # && adduser ${USER_NAME} sudo                             \
        # && passwd -d ${USER_NAME}                                \
        # && sed -E -i 's|^#?(PasswordAuthentication)\s.*|\1 no|' /etc/ssh/sshd_config \
        # && if ! grep '^PasswordAuthentication\s' /etc/ssh/sshd_config; then echo 'PasswordAuthentication no' |sudo tee -a /etc/ssh/sshd_config; fi \
    )
# RUN systemctl enable sshd
# RUN sed -i 's/PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config \
#     && sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
# systemctl start sshd

# Notes
#   Skip password on host side:
#   $ sudo apt-get install sshpass
#   $ sshpass -p your_password ssh user@hostname
