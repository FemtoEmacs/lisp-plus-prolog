# wamdocs

The wamcompiler.pdf is a tutorial on wamcompiler, which is a wam based Prolog embedded in Common
Lisp. It discusses both Lisp and Prolog programming. There is also a tutorial on Scheme plus
Prolog programming -- lispp.pdf

# Compiling the tutorials
The tutorials were written in latex, and the distribution is in pdf. Therefore you
don't need to compile the tutorials yourself. However, if you want to compile the
tutorial, you need a fairly complete installation of latex. The building process
is described below:

1. Compilation of the Scheme tutorial:

```shell
$ pdflatex lispp.tex
$ pdflatex lispp.tex
$ makeindex lispp.idx
$ pdflatex lispp.tex
$ pdflatex lispp.tex
```

You need to compile at list twice, so LaTeX will fix citations, references and indexes.

2. Compilations of the Lisp tutorial.

```shell
$ pdflatex wamcompiler.tex
$ pdflatex wamcompiler.tex
$ makeindex wamcompiler.idx
$ pdflatex wamcompiler.tex
$ pdflatex wamcompiler.tex
```

# Executing Prolog programs

You need Racket or Chez Scheme installed in your machine. In the case of Chez
Scheme, you will need to compile from sources. Ask for help from a Computer
Science major, if you are not conversant with computers. There are Racket
binaries for almost any machine. However, you may want to create a symbolic
link for Racket. In the case of MacIntosh, this is done as shown below:

```shell
› sudo ln -s /Applications/Racket\ v7.5/bin/racket /usr/local/bin/racket
Password: ****

› racket
Welcome to Racket v7.5.
> (exit)
```

The next step is to build the library. Enter the `sxprolog` folder:

```shell
~/lisp-plus-prolog/sxprolog› make lib
... stuff
~/lisp-plus-prolog/sxprolog› cp libcxprolog.so libcxprolog.dylib
```

The last step is necessary only if you are working on the Macintosh,
and prefer using Racket over Chez Scheme.

You must always launch Prolog from a folder where the libcxprolog is
present. If you want to launch Prolog from anywhere, you need to
install the library in your $PATH. For instance, copy it to `/usr/local`
as shown below:

```shell
$ sudo cp ~/lisp-plus-prolog/sxprolog› cp libcxprolog.so /usr/local/lib/
```

Of course, if you are using Macintosh, you must copy `libcxprolog.dylib`
instead of `libcxprolog.so`

If you do not wart a global installation, you must launch Scheme from
the `sxprolog` folder, as I told before. Suppose you want to execute
`fvalue.pl` from the `sxprolog` folder. In that case, you can do it thus:

```shell
› scheme
Chez Scheme Version 9.5.3
Copyright 1984-2019 Cisco Systems, Inc.

> (load "prolog.scm")
> (logic "-f prolog-src/fvalue.pl")
CxProlog version 0.98.2 [development]

[main] ?- futval(10000, 8, N, FV).
N = 2
FV = 11664 ? ;
N = 4
FV = 13604.8896 ? ;
N = 8
FV = 18509.3021 ? .
...
?- halt.
% CxProlog halted.
0
> (exit)
```










