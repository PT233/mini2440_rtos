# 使用 Ubuntu 22.04 作为基础镜像
FROM ubuntu:22.04

# 避免安装过程中的交互式提示
ENV DEBIAN_FRONTEND=noninteractive

# 配置 apt 重试次数，增加下载成功率
RUN echo "Acquire::Retries \"10\";" > /etc/apt/apt.conf.d/80-retries

# 更换为网易 163 镜像源
RUN sed -i 's/archive.ubuntu.com/mirrors.163.com/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.163.com/g' /etc/apt/sources.list && \
    sed -i 's/mirrors.tuna.tsinghua.edu.cn/mirrors.163.com/g' /etc/apt/sources.list && \
    sed -i 's/mirrors.aliyun.com/mirrors.163.com/g' /etc/apt/sources.list

# 1. 更新软件源 & 安装基础工具
RUN apt-get update && apt-get install -y --fix-missing \
    make \
    libusb-0.1-4 \
    && rm -rf /var/lib/apt/lists/*

# 2. 单独安装 ARM 工具链 (大文件，单独分层构建)
RUN apt-get update && apt-get install -y --fix-missing \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    && rm -rf /var/lib/apt/lists/*

# 复制 dnw 工具到容器中
COPY dnw /usr/local/bin/dnw
RUN chmod +x /usr/local/bin/dnw

# 设置容器内的工作目录
WORKDIR /app

# 默认命令：当运行容器且未指定命令时，默认执行 make
CMD ["make"]
