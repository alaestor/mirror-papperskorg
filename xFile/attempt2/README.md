# xFile

As the `xfile.h` header says:
```
T H E         ,.__                            XXXXXxX  xxxXXXXXX
             ((-\)),                             xXX      xXX
             | . . {         ___.                  X      XX
T R U T H    (  _) )       /(()-/)                  X   xX
              \ _ /       / /. . |\                 xx xX
            __-\_/-__     ||  ]  )|        T  H  E   (X)   F  I  L  E
I S        / \ \@//  \    /|  -  ||                 xX xx
          |  | |{||__|\   /_\___/|,                Xx   Xx
          |  | |}||  ||  |    \/   \              XX     XX   (TM)
O U T     |  {.|{/|  ||  | |_| __| |            xXX       XX
          |  /.|__|  ||  \___\_____/         xXXXXXXx   XXXXxXx
           \/ .|=>|  |/   /   o|  \
T H E R E .exe
```

**The xFile utility** uses the Windows Resources system and a payload child binary
to store arbitrary persistent data inside the executible itself that would
otherwise need to be an external file. For example, a simple settings.ini file.
In theory, xFile could also be used to implement on-disk polymorphism of
executable code by the parent writing modified code to itself as data and then
loading it into an executable memory region after it's been launched again.

**The need for a child "payload" binary** is due to windows locking executable
files while they're open, meaning its impossible for a program to write to
itself. A temporary child writer payload executable is written out, updated
with changes to be made, and launched as the parent closes. It waits for the
parent process to fully close, writes it's changes to the parent file, and then
closes and deletes itself. Embedding a child binary within the parent **can
cause many false-positive detections in security software** so it's unlikely
that this utility could be of any practical or large-scale use. Having a payload
is an alternative to the parent copying itself and launching itself with special
parameters. Though this would eliminate some false-positive malware detection;
using a payload binary lets us not worry about writing superfluous data copying
and writing if the parent file is large. It also lets the parent not have to
handle special launch parameters.

This project was just a curiousity and creating it was very educational.
It's very reason for existence - *removing the need for external files by
containing persistent arbitrary data within the executible file itself* - is a
complete non-issue. **This utility, in fact, has very little utility. It's
entirely useless, pointless, and is an over-complicated solution to an
otherwise non-existent problem.** As the only person who mattered in my life
once said: 

> Hey... I see you're trying to store stuff.
> So... there's this thing... called... a ***file***? Heard of it?

There is still much that could be improved, but no.

# xFile Static Library

It's hoped that xFile could be adapted into a static library to more easily be
implemented in projects. However, this was originally a *Future Gadget
Laboratory* project, and since it's destruction the sole maintainer to this
monstrosity has lost all will live, as well as to improve and support this
needless utility.

# A Working Example

can be found in the form of the `/x_parent/main.cpp`. A proper unit-test has
yet to be added, and likely never will be. This is a bad example as it isn't
very barebones, the compilation steps are complicated due to embedding the
child payload, and the utility still has much work needed before it could be
called anything close to a "release".

# Want To Use xFile In Your Project?
If you find yourself in need of this utility, or plan to implement it in your
project

- **First**: may god have mercy on your soul

- **Second**: you should let me know via [my discord](https://discordapp.com/invite/bEPdcMV).