# OpenSptr
A recreation of the Nintendo Ds game The Legend of Zelda Spirit Tracks <br>

I'm using c++ and [SDL2](https://github.com/libsdl-org/SDL) to get access to audio, video and controls. <br>
Big thanks to: <br>

<a href="https://problemkaputt.de/gbatek.htm" target="_blank">Problemkaputt.de</a>
<a href="https://www.feshrine.net/hacking/doc/nds-sdat.html" target="_blank">Feshrine.net</a>
<a href="https://github.com/vgmtrans/vgmtrans" target="_blank">VgmTrans</a>

This is work in progress and could take years to finish! <br>
I've never created or reverse engineered a game before so don't expect too much. <br>

## TODO: <br>
- Maybe reworking byteutils.cpp
- Add loading raw SWAV sample and later pitch in sound mixer 

## Current state: <br>

- [x] Reading/unpacking Rom Fat filesystem
- [x] Reading/unpacking SDAT Sound archives
- [x] Reading/Playing   STRM Sound files
- [x] Reading           SBNK BANK files
- [x] Reading/Playing   SWAV Sample files
- [ ] Reading/Playing   SSEQ/Midi files
- [ ] Reading/Playing   SSAR files
- [ ] Reading/Unpacking Archives <br>
...
