#install.sh
Password="123456"

#change mode
chmod 777 berxel-usb.rules

#move rules to /etc/udev/rules.d
echo $Password | sudo -S cp berxel-usb.rules /etc/udev/rules.d
echo $Password | sudo -S udevadm control --reload-rules