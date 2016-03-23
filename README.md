# 4d-plugin-midi
Tmidity wrapper plugin for OS X

##Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🚫|🚫|

Commands
---

```
long:=MIDI Play (text)
MIDI ABORT (long)
MIDI SUSPEND (long)
MIDI RESUME (long)
```

Discussion
---

Pass the full path to your MIDI file in ``MIDI Play``. It will start playing and return a reference number. The reference number is incidentally the process ID of the spawned Tmidity process. You can suspend, resume or abort the tune with this number. All running processes are aborted when you close the application.

By default, the sound font files distributed with the homwbrew version of Timidity ``2.14.0`` is used. You should be able to edit the ``timidity.cfg`` file to customise the sound. 

**TODO**: The 2nd paramter for ``MIDI Play`` is not implemented. The idea is to accept any command line arguments for Timidity here.
