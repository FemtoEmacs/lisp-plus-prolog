# Modifications of CxProlog
As required by the GNU General Public License
as published by the Free Software Foundation,
this file describes the modifications of
CxProlog so it can be used as part of the
lisp+prolog project. CxProlog was developed
by Artur Miguel Dias from the Universidade Nova
de Lisboa. The modifications done by the maintainers
of this repository were very small:

(1) The definition of int `repl(int i, char *str)`
    was added to the file `sxprolog/src/CxProlog.c`
(2) The name of the distribution was changed to
    sxprolog, in order to avoid confusion with the
    unmodified original package.
(3) A small interface between Chez Scheme and Prolog
    was added to the distribution package.
(4) Programs in Java were removed from this distribution,
    because many firms and Internet services do not accept
    archives with Java content. People who need to work
    with Java must use the original package with the
    addition of the modified files, sxprolog/toprolog.scm
    and sxprolog/src/CxProlog.c

## Building the dependencies
All pieces of software discussed here are very easy
to compile, or have binaries available: Chez Scheme,
Emacs text editor and CxProlog. In the case of CxProlog,
the execution of the line below from a terminal should
be sufficient:

```shell

$ make lib
$ cp libcxprolog.so libcxprolog.dylib

```

Be sure that you are inside the ~/sxprolog/ folder. You need to
copy libcxprolog.so to libcxprolog.dylib only if you are working
with a MacIntosh computer and needs to use the Racket language.

## Test Prolog for Racket
CxProlog for Scheme and Common Lisp does not work on the Racket IDE.
I only succeed to make Racket work on a terminal, at a command line.

```shell
$ racket
Welcome to Racket v7.5.
> (require "prolog.rkt")
> (logic "I am not from here")
CxProlog version 0.98.2 [development]

[main] ?- os_args(X).
X = ['I',am,not,from,here]

[main] ?- halt.
% CxProlog halted.
0
> (logic "She walks in beauty like the night")

[main] ?- os_args(X).
X = ['She',walks,in,beauty,like,the,night]

[main] ?- halt.
% CxProlog halted.
0
> (exit)

$ _
```

In the case of a MacIntosh computer, you will may want
to link racket to a call from /usr/local/bin for greater
convenience. Here is how to do it:

```shell

$ sudo ln -s /Applications/Racket\ v7.5/bin/racket \
              /usr/local/bin/racket

```

## Test Prolog for Chez Scheme
The foreign language interface of Chez Scheme is quite
complex. I would suggest the adoption of a simpler
interface, such as the one found in Chez Scheme. Anyway,
here is the test of chezprolog:

```shell
› scheme
Chez Scheme Version 9.5.3
Copyright 1984-2019 Cisco Systems, Inc.

> (load "chezprolog.scm")
> (logic "She walks in beauty like the night")
CxProlog version 0.98.2 [development]

[main] ?- os_args(X).
X = ['She',walks,in,beauty,like,the,night]

[main] ?- halt.
% CxProlog halted.
0
> (exit)
```

## Common Lisp
For the time being, the only option for Common Lisp is
wamcompiler. I already filed an issue on wamcompiler
page. I also asked Lucas and Pedro to create a Common
Lisp interface for cxprolog.

