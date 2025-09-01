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
        # mkdir /var/run/sshd && \
        # echo '${USER_NAME} ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
        # sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
        # sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config && \
        # echo "AllowUsers ${USER_NAME}" >> /etc/ssh/sshd_config
        # && adduser --disabled-password --gecos '' ${USER_NAME}   \
        # && adduser ${USER_NAME} sudo                             \
        # && passwd -d ${USER_NAME}                                \
        # && sed -E -i 's|^#?(PasswordAuthentication)\s.*|\1 no|' /etc/ssh/sshd_config \
        # && if ! grep '^PasswordAuthentication\s' /etc/ssh/sshd_config; then echo 'PasswordAuthentication no' |sudo tee -a /etc/ssh/sshd_config; fi \
    )

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
# systemctl start sshd

# Notes
#   Skip password on host side:
#   $ sudo apt-get install sshpass
#   $ sshpass -p your_password ssh user@hostname
