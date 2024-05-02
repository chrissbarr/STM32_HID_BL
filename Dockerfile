FROM ubuntu:22.04

ARG ARM_VERSION=12.3.rel1
ARG TOOLS_PATH=/opt/gcc-arm-none-eabi
ARG HOST_ARCH=x86_64

RUN apt-get update && apt-get install -y \
	build-essential \
	cmake \
	ninja-build \
	curl \ 
	git


# Get ARM Toolchain
RUN echo "Downloading ARM GNU GCC for Platform: $HOST_ARCH" \
	&& mkdir ${TOOLS_PATH} \
	&& curl -Lo gcc-arm-none-eabi.tar.xz "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_VERSION}/binrel/arm-gnu-toolchain-${ARM_VERSION}-${HOST_ARCH}-arm-none-eabi.tar.xz" \
	&& tar xf gcc-arm-none-eabi.tar.xz --strip-components=1 -C ${TOOLS_PATH} \
	&& rm gcc-arm-none-eabi.tar.xz \
	&& rm ${TOOLS_PATH}/*.txt \
	&& rm -rf ${TOOLS_PATH}/share/doc \
	&& echo "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_VERSION}/binrel/arm-gnu-toolchain-${ARM_VERSION}-${HOST_ARCH}-arm-none-eabi.tar.xz"

# Add Toolchain to PATH
ENV PATH="$PATH:${TOOLS_PATH}/bin"