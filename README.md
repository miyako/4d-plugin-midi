# 4d-plugin-midi
Play MIDI files (Tmidity wrapper).

##Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🆗|🆗|

Commands
---

```
long:=MIDI Play (text;text array)
MIDI ABORT (long)
MIDI SUSPEND (long)
MIDI RESUME (long)
long:=MIDI Is running (long)
```

Discussion
---

Pass the full path to your MIDI file in ``MIDI Play``. It will start playing and return a reference number. The reference number is incidentally the process ID of the spawned Tmidity process. You can suspend, resume or abort the tune with this number. All running processes are aborted when you close the application.

By default, the sound font files distributed with the homwbrew version of Timidity ``2.14.0`` is used. You should be able to edit the ``timidity.cfg`` file to customise the sound. 

Optinally, you can pass any timidity argument to the 2nd paramter for ``MIDI Play``.

Example
---

```
  //pass command line options to tmidity 
ARRAY TEXT($args;2)
$args{1}:="F"
$args{2}:="chorus=s,80"

$m:=MIDI Play (Get 4D folder(Current resources folder)+"sample.mid";$args)

  //new in 1.1
While (MIDI Is running ($m)=1)

DELAY PROCESS(Current process;10)

End while 

  //MIDI SUSPEND ($m)
  //MIDI RESUME ($m)

MIDI ABORT ($m)

ALERT("fin")
```
