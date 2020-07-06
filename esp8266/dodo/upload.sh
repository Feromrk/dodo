SRC_FILE="dodo.ino"
FW_FILE="dodo.ino.nodemcu.bin"
FW_VERSION=$(grep "const unsigned int FW_VERSION" dodo.ino | sed -e "s/^const unsigned int FW_VERSION = //" -e "s/;$//")

if [[ ! -f "$FW_FILE" ]]; then
    echo "$FW_FILE does not exist; exiting"
    exit 1
fi

mv -i -v "$FW_FILE" "$FW_VERSION.$FW_FILE"
scp "$FW_VERSION.$FW_FILE" root@192.168.2.117:/var/opt/dodo/sensor_firmware/