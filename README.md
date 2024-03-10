
# CS2 Kernel cheat

A simple CS2 cheat in using a kernel driver


## Features

- Simple triggerbot with autostop(you need to uncomment it at line 285 in the usermode/src/main.cpp)
- Simple bhop 
- Hitsounds
- Kernelmode to be undetectable by vac


## Compiling

Open it in visual studio and build as debug or as release whatever you like obv in x64

You will also need the sfml libraries for this to compile and run
## How to use it

You need to map the driver using soemthing like [KDmapper](https://github.com/TheCruZ/kdmapper) 

Then you need to run CS2 and open the usermode.exe.

Windows Defender probaly needs to be off but you can just whitelist the diretory 

Disable vulnerable driver list if enabled [Guide for that](https://support.microsoft.com/en-au/topic/kb5020779-the-vulnerable-driver-blocklist-after-the-october-2022-preview-release-3fcbe13a-6013-4118-b584-fcfbc6a09936)
## Note

I know this is horrible code and also horribly formatted because most is done in 1 file i am sorry for that.

## Future plans
- Currently the bhop isnt the best as it sometimes just fails to jump so i will improve that.
- I am also trying to get add a precision enhanced automatic target acquisition and engagement facilitation system protocol but i cant really get it to work 

## Updating

I cant guarentee that i will update this for every CS2 update so you can update the offsets yourself using [Cs-Dumper](https://github.com/a2x/cs2-dumper) and then just replace your current client.dll.hpp with the one by cs2-dumper and do the same for offsets.hpp
## Authors
- [@maxiwee69](https://www.github.com/maxiwee69) made most of the actuall cheat part
- [@cazzwastaken](https://www.github.com/cazzwastaken) made the driver and the the way the usermode communicates with the driver + he made the bhop

## Downloading

You should compile yourself bc i probaly will sometimes forget to update the release but you can also get it from the release or from [here](https://maxiwee.de/cs2.rar)


## Contributing

Contributions are always welcome via a pull request!
