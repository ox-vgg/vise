#!/bin/bash

USER_ID=${LOCAL_UID:-1000}
GROUP_ID=${LOCAL_GID:-$USER_ID}

id -g "${GROUP_ID}" &>/dev/null || groupadd --gid ${GROUP_ID} user
id -u "${USER_ID}" &>/dev/null || useradd \
    --uid ${USER_ID} \
    --gid ${GROUP_ID} \
    --create-home \
    --home /home/user \
    --shell /bin/bash \
    --comment "user" \
    user

export HOME=/home/user
echo "Starting with UID: ${USER_ID}, GID: ${GROUP_ID}"

exec /usr/sbin/gosu user "$@"