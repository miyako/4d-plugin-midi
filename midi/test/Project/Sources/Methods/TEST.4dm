//%attributes = {}
  //pass command line options to tmidity 
ARRAY TEXT:C222($args;2)
$args{1}:="F"
$args{2}:="chorus=s,80"

$m:=MIDI Play (Get 4D folder:C485(Current resources folder:K5:16)+"sample.mid";$args)

Repeat 
	
	DELAY PROCESS:C323(Current process:C322;10)
	
Until (MIDI Is running ($m)#1)

  //MIDI SUSPEND ($m)
  //MIDI RESUME ($m)

MIDI ABORT ($m)

ALERT:C41("fin")