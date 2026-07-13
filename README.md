# channels.conf_builder
A simple C application to build a channels.conf file for dvbv5-zap using AltDVB format .INI transponder lists. Instead of laboriously transcribing a data transponders tuning parameters into your channels.conf for capturing with dvbv5-zap, wouldn't it be nice if we could automate the process? Your boy Rob has got your covered! Tested and working on DragonOS FocalX R37. WARNING: Coded with AI, use at your own risk!

## DOWNLOAD AND BUILD

```bash
cd ~/
wget https://raw.githubusercontent.com/RobVK8FOES/channels.conf_builder/refs/heads/master/channels.conf_builder.cpp
g++ -O3 -Wall channels.conf_builder.cpp -o channels.conf_builder
```

## PREPARATION

Use EBS Pro in Windows to find IP data transponders, and add them to the 'favourites' list:

![](/images/1.png)

Create a AltDVB formatted .INI transponder file of 'favourites' list:

![](/images/2.PNG)

## HOW TO USE

Launch the binary to see help file:

```text
Usage: ./channels.conf_builder <input1.ini> [input2.ini ...] <output_channels.conf>
```

Example Usage:

```text
./channels.conf_builder 1340.ini channels.conf
```

./channels.conf_builder = Name of binary

1340.ini = AltDVB format .INI transponder list file name (specify as many as you want, separated by a space)

channels.conf = Name of dvbv5-zap compatible 'channels.conf' file

![](/images/3.PNG)

## INSTALLATION AND CLEANUP (OPTIONAL)

```text
sudo cp -v channels.conf_builder /usr/local/bin/channels.conf_builder
rm -v channels.conf_builder.cpp channels.conf_builder
```
