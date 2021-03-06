#!/bin/bash
echo "This script needs your password to copy guarddog startup script to the system ..!"
echo "you can also use sysv-rc-conf to see if the guarddog script is called on the runlevel of your choice !"
echo "This is supposed to be run from the root directory of the RoboVision Project"

echo
echo -n "                Do you want to proceed (Y/N)?"
read answer
if test "$answer" != "Y" -a "$answer" != "y";
then exit 0;
fi
 
sudo usermod -a -G audio,video guarddog 
sudo cp Scripts/guarddog_linux_startup_script /etc/init.d/guarddog_linux_startup_script
sudo chmod 755 /etc/init.d/guarddog_linux_startup_script
sudo chmod +x /etc/init.d/guarddog_linux_startup_script
sudo update-rc.d guarddog_linux_startup_script defaults 

#Add a guarddog command that will relay to the RoboVision process
sudo cp Scripts/guarddog /bin/guarddog
 


echo "The next segment gives guarddog an IP of 192.168.1.14 and gateway 192.168.1.1 on ethernet ..!"
echo "and an IP of 192.168.2.14 and gateway 192.168.2.1 on wifi ..!"
echo "( also puts it on a wireless access point mode ! ) "
echo
echo -n "                Do you want to proceed (Y/N)?"
read answer
if test "$answer" != "Y" -a "$answer" != "y";
then exit 0;
fi
 
 
sudo cp Scripts/interfaces /etc/network/interfaces 















exit 0
