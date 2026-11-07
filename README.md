![version](https://img.shields.io/badge/version-17%2B-3E8B93)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)[![license](https://img.shields.io/github/license/miyako/4d-plugin-midi)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/4d-plugin-midi/total)

# 4d-plugin-midi

Play MIDI files from 4D. The plugin does not synthesise sound itself: it launches the TiMidity++ 2.14.0 command-line player that ships inside the plugin bundle (`timidity` on macOS, `timidity.exe` on Windows) as a separate process, and gives you a `Longint` reference to that process so you can pause, resume, poll and stop it. The instruments are the FreePats patches in `midi.bundle/Contents/Resources/timidity/`, configured by `timidity.cfg` in the same folder.

| Command | Returns | Purpose |
|---|---|---|
| [MIDI Play](#midi-play) | `Longint` | Start playing a MIDI file in a new TiMidity++ process |
| [MIDI ABORT](#midi-abort) | | Stop a playback and forget its reference |
| [MIDI SUSPEND](#midi-suspend) | | Pause a playback |
| [MIDI RESUME](#midi-resume) | | Continue a paused playback |
| [MIDI Is running](#midi-is-running) | `Longint` | `1` while the playback process is alive, else `0` |

**Platforms:** macOS (Intel and Apple Silicon), Windows 64-bit. 4D v17 or later.

Releases: [github.com/miyako/4d-plugin-midi/releases](https://github.com/miyako/4d-plugin-midi/releases)

---

## Requirements & platform notes

- **Apple Silicon needs Rosetta 2.** The plugin itself is universal, but the bundled `timidity` helper is an Intel (`x86_64`) binary. Without Rosetta the helper cannot start, and [MIDI Play](#midi-play) plays nothing (inferred from the binary's architecture; not tested on a Mac without Rosetta).
- **Failure is silent.** No command raises a 4D error. A file that does not exist or is not a MIDI file still starts a TiMidity++ process, which exits at once; you see it only as [MIDI Is running](#midi-is-running) returning `0`.
- **Both parameters of [MIDI Play](#midi-play) are mandatory.** Pass an empty text array when you have no options.
- **The reference is a process ID.** On Windows, [MIDI Play](#midi-play) returns the process ID of `timidity.exe`. **On macOS** it currently returns `0` for every playback (see the [MIDI Play](#midi-play) description), so only one playback at a time can be controlled there.
- **Always end with [MIDI ABORT](#midi-abort).** A playback that finished on its own stays registered (and on Windows keeps its process handle open) until you call [MIDI ABORT](#midi-abort) with its reference.
- **Thread-safe.** All five commands are declared thread-safe and can be called from preemptive processes.
- **Quitting 4D stops the music.** When the plugin is unloaded, every registered playback that is still running is terminated.

---

## MIDI Play

### Syntax

```
ref := MIDI Play ( path ; options )
```

| Parameter | Type | Description |
|---|---|---|
| `path` | Text | Full path of the MIDI file, in the platform's system path format (see below) |
| `options` | Text array | Extra TiMidity++ command-line arguments, one per element; empty elements are skipped |
| Result | Longint | Reference of the playback (see below); `0` if the process could not be started |

### Description

Starts the bundled TiMidity++ player on `path` and returns at once; the music plays in the background while your method continues. The player runs with `midi.bundle/Contents/Resources/timidity/` as its working directory and reads `timidity.cfg` from there, so you can change the instruments by editing that file (a change inside the bundle breaks the bundle's code signature; re-sign it if your deployment checks signatures).

**On macOS**, `path` must be a 4D system path (colon-separated, as returned by `Get 4D folder`); the plugin converts it to a POSIX path itself. A POSIX path (`/Users/...`) is not recognised. The command line is `timidity -c timidity.cfg <file> <options...>`, each array element being one separate argument, passed verbatim.

**On Windows**, `path` is a normal Windows path. The command line is `timidity.exe "<file>" -c timidity.cfg <options...>`: the elements are appended separated by single spaces without quotes, so an element that contains a space becomes several arguments.

Every element of `options` is used, including element `0`, as long as it is not empty. The plugin adds nothing to the elements: write each argument exactly as TiMidity++ expects it, check the TiMidity++ documentation for the available options.

**The result.** On Windows it is the process ID of `timidity.exe`, or `0` if `CreateProcess` failed. **On macOS** the result is always `0`: the plugin registers the playback under the process ID that `NSTask` reports before launching, which is `0`, and never returns the real one. The consequences on macOS:

- [MIDI Is running](#midi-is-running), [MIDI SUSPEND](#midi-suspend), [MIDI RESUME](#midi-resume) and [MIDI ABORT](#midi-abort) called with `0` act on the playback registered under `0`.
- If you call [MIDI Play](#midi-play) again before [MIDI ABORT](#midi-abort) of the previous playback, the new playback still starts, but it cannot be registered: no command can pause, poll or stop it, and it is not stopped when 4D quits (it ends when the file ends).

So on macOS, call [MIDI ABORT](#midi-abort) before starting the next file.

If the helper cannot be launched on macOS (missing from the bundle, or no Rosetta on Apple Silicon), the command returns `0` and nothing plays.

### Example

From the plugin's own test method (`test/Project/Sources/Methods/TEST.4dm`):

```4d
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
```

Play without options and stop after ten seconds:

```4d
ARRAY TEXT($options;0)
$ref:=MIDI Play(Get 4D folder(Current resources folder)+"sample.mid";$options)
DELAY PROCESS(Current process;60*10)
MIDI ABORT($ref)
```

---

## MIDI ABORT

### Syntax

```
MIDI ABORT ( ref )
```

| Parameter | Type | Description |
|---|---|---|
| `ref` | Longint | Reference returned by [MIDI Play](#midi-play) |
| Result | | None |

### Description

Stops the playback if it is still running, frees what the plugin holds for it, and forgets `ref`. Afterwards [MIDI Is running](#midi-is-running) returns `0` for that reference. An unknown reference is ignored.

**On macOS** the process is asked to terminate (`SIGTERM`). **On Windows** it is killed with `TerminateProcess`.

Call it also for a playback that already ended on its own, to release it.

### Example

```4d
ARRAY TEXT($options;0)
$ref:=MIDI Play($path;$options)
  // ...
MIDI ABORT($ref)  // stops it if still playing, and releases it in any case
```

---

## MIDI SUSPEND

### Syntax

```
MIDI SUSPEND ( ref )
```

| Parameter | Type | Description |
|---|---|---|
| `ref` | Longint | Reference returned by [MIDI Play](#midi-play) |
| Result | | None |

### Description

Pauses the playback. Resume it with [MIDI RESUME](#midi-resume). An unknown reference is ignored.

**On macOS** the whole process is stopped (`NSTask suspend`, i.e. `SIGSTOP`). **On Windows** only the main thread of `timidity.exe` is suspended (`SuspendThread`); whether that silences all audio output depends on how TiMidity++ uses threads (not verified).

[MIDI Is running](#midi-is-running) still returns `1` for a suspended playback.

### Example

```4d
MIDI SUSPEND($ref)
DELAY PROCESS(Current process;60*3)  // three seconds of silence
MIDI RESUME($ref)
```

---

## MIDI RESUME

### Syntax

```
MIDI RESUME ( ref )
```

| Parameter | Type | Description |
|---|---|---|
| `ref` | Longint | Reference returned by [MIDI Play](#midi-play) |
| Result | | None |

### Description

Continues a playback paused with [MIDI SUSPEND](#midi-suspend). An unknown reference is ignored.

**On macOS** this is `NSTask resume`. **On Windows** it is `ResumeThread` on the main thread; Windows counts suspensions, so a thread suspended twice needs two calls (documented behaviour of `ResumeThread`).

### Example

See [MIDI SUSPEND](#midi-suspend).

---

## MIDI Is running

### Syntax

```
running := MIDI Is running ( ref )
```

| Parameter | Type | Description |
|---|---|---|
| `ref` | Longint | Reference returned by [MIDI Play](#midi-play) |
| Result | Longint | `1` if the playback process is still alive, `0` if it has ended, was aborted, or `ref` is unknown |

### Description

Use it to wait for the end of a file. A paused playback counts as running. Because a bad file makes TiMidity++ exit immediately, a `0` right after [MIDI Play](#midi-play) usually means the file could not be played.

### Example

```4d
While (MIDI Is running($ref)=1)
	DELAY PROCESS(Current process;10)
End while 
MIDI ABORT($ref)
```

---

## Error handling & troubleshooting

- **Nothing plays and no error.** Check the path format: a system path with colons on macOS, a Windows path on Windows. Then check that [MIDI Is running](#midi-is-running) returns `1` right after [MIDI Play](#midi-play); `0` means TiMidity++ exited or never started.
- **Nothing plays on an Apple Silicon Mac.** Install Rosetta 2; the bundled `timidity` is Intel-only.
- **[MIDI Play](#midi-play) returns 0 on macOS.** This is the current behaviour for every playback on macOS, not an error. Use the returned value anyway, and call [MIDI ABORT](#midi-abort) before the next [MIDI Play](#midi-play).
- **A second file on macOS cannot be stopped.** It was started while the previous playback was still registered, so no reference points to it and quitting 4D does not stop it; it plays to the end. Call [MIDI ABORT](#midi-abort) on the previous playback before starting another.
- **An option with spaces is ignored or misread on Windows.** Elements are not quoted on Windows; split the option into separate elements, or avoid spaces.
- **Changing the sound.** Edit `Contents/Resources/timidity/timidity.cfg` in the bundle, and re-sign the bundle if needed.
- **Resources not released.** Every reference must end with [MIDI ABORT](#midi-abort), even after the music stopped by itself.

---

## Quick reference

```4d
ARRAY TEXT($options;0)
$ref:=MIDI Play($systemPathToMidiFile;$options)
MIDI SUSPEND($ref)
MIDI RESUME($ref)
While (MIDI Is running($ref)=1)
	DELAY PROCESS(Current process;10)
End while 
MIDI ABORT($ref)
```
