#!/usr/bin/env bash

add-apt-repository -y ppa:terry.guo/gcc-arm-embedded
apt-get update
apt-get install -y pcregrep libpcre3 python3 git gcc-arm-none-eabi gcc-msp430 unzip \
    qemu-system-x86 g++-multilib gcc-multilib build-essential gcc-avr binutils-avr avr-libc \
    doxygen cppcheck python-setuptools libusb-1.0-0 libusb-1.0-0-dev libftdi1 libftdi-dev \
    libftdipp-dev libftdipp1 libhidapi-dev libhidapi-hidraw0 libhidapi-libusb0 \
    make cmake autotools-dev autoconf pkg-config jimsh libtool valgrind

# This package contains usbserial kernel drivers
# see http://ubuntu.5.x6.nabble.com/PATCH-0-2-linux-image-extra-support-td365008.html
apt-get -y install linux-image-extra-virtual

apt-get -y upgrade

# give permissions for serial ports
adduser vagrant dialout

# install openocd
if ! hash openocd 2>/dev/null; then
    wget -qO - https://launchpad.net/ubuntu/+archive/primary/+files/openocd_0.9.0.orig.tar.gz | tar xz
    cd openocd-0.9.0
    ./configure --enable-amtjtagaccel --enable-armjtagew --enable-buspirate --enable-ftdi \
        --enable-gw16012 --enable-jlink --enable-oocd_trace --enable-opendous --enable-osbdm \
        --enable-parport --enable-presto_libftdi --enable-remote-bitbang --enable-rlink \
        --enable-stlink --enable-ti-icdi --enable-ulink --enable-usbprog --enable-vsllink \
        --enable-aice --enable-cmsis-dap --enable-dummy --enable-jtag_vpi --enable-openjtag_ftdi \
        --enable-usb-blaster-2 --enable-usb_blaster_libftdi
    make && make install && cd .. && rm -rf openocd-0.9.0
fi

if ! hash st-util 2>/dev/null; then
    git clone --depth 1 https://github.com/texane/stlink stlink.git && cd stlink.git
    ./autogen.sh && ./configure && make && make install
    cd .. && rm -rf stlink.git
fi

# install cli-tools
if ! hash experiment-cli 2>/dev/null; then
    wget -qO - https://github.com/iot-lab/cli-tools/archive/1.6.0.tar.gz | tar xz
    cd cli-tools-1.6.0 && python setup.py install && cd .. && rm -rf cli-tools-1.6.0
fi

# configure git
git config --global user.email "riot-os_vagrant@example.com"
git config --global user.name "RIOT-OS Vagrant"

# create a symbolic link to the RIOT-OS directory
if ! [ -L /home/vagrant/RIOT-OS ]; then
    ln -fs /vagrant /home/vagrant/RIOT-OS
    chown -h vagrant:vagrant /home/vagrant/RIOT-OS
fi

# copy udev rules
cp -f RIOT-OS/dist/tools/vagrant/udev_rules/*.rules /etc/udev/rules.d/
udevadm control --reload-rules ; udevadm trigger

# cleanup
apt-get -y autoremove
