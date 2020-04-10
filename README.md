# en.UTF-8

This is a fork from tapir ( https://github.com/qnighy/tapir ), mostly to clean stuff and let it ready for hacking. Someday it will be enhanced up to the limit of portability, as RGSS is inherently Windows API stuff, one can just hope that people don't use external libraries, because when they do, this stuff here is just bullshit and not even WINE can help you...

To compile this stuff, tapir used to hold submodules and pack the object files from ruby together, inside its tree. We use the more "pragmatic" approach, with each ruby version being used as libraries. Another thing that I do not recommend is using the scripts to install the Run Time Packages needed, because the addresses change often and you have to register in the official "western" version. The scripts are kept merely for curiosity, go grab all the stuff inside TKool site ( https://tkool.jp/ ) because it is faster and you won't have problems with links. The XP package is a little different, so you have to cut some directories.

## Instructions.

Hooray! Before anything, you should bring along some extracting tool, like the one in "rgssad.c". It won't work with packages anymore, they are dumb-made and delay our positive attitude of sharing. It is a good idea to convert every possible file to lower or UPPER case, to cease filename issues that you may have, the worst offender being "IconSet". Seems that you can also type filenames...

The incompatibility rate IS HIGH, things may break before you can reach menus or inside them, so you won't even save stuff. There is the concrete ruby piece, as it is the same thing that processes most stuff, but there is also the main scripting that is not complete, and the "functionality" that is nowhere near that...
