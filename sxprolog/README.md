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
```

Be sure that you are inside the ~/sxprolog/ folder.

