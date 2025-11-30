#!/bin/bash
#./run_docker.sh
#./run_docker.sh make clean
#
# 镜像名称
IMAGE_NAME="mini2440-env"

# 检查镜像是否存在，不存在则构建
if [[ "$(docker images -q $IMAGE_NAME 2> /dev/null)" == "" ]]; then
    echo "镜像 $IMAGE_NAME 不存在，正在构建..."
    docker build -t $IMAGE_NAME .
fi

# 运行容器
# --rm: 退出后删除容器
# --privileged: 赋予特权 (用于 dnw 访问 USB)
# -v "$PWD":/app: 挂载当前目录
# "$@": 传递给脚本的所有参数 (例如 make, make clean, dnw xxx)
docker run --rm --privileged -v "$PWD":/app $IMAGE_NAME "$@"
