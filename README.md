# Lisp as a tool for fighting sofware obsolescence
The subject of this repository is not Lisp, but sofware obsolescence.
Lisp is the only computer language that does not become obsolete. In
my long life as a computer scientist, I coded in many languages, PLC, PLI,
APL, Algol, Pascal, Forth, Trilogy, Clean, C and Lisp. Now I program
only in Lisp and C, not because I have a strong preference for these languages,
but because they are the only language that I learned, and are still actively
maintained. My message is: Artificial Intelligence must be coded in Lisp for
a simple reason: Lisp is the only language that does not become obsolete.

Discover how Intelligence works is a tough endeavor. It is like to build a
Cathedral, the team must be prepared to work for centuries. The construction
of the Duomo di Millano started in 1386, when Gian Galeazzo Visconti was
raised to power in Milan. Simone da Orsenigo was the first engineer to work
in the famous cathedral. In 1389, the duc of Milan hired the French engineer
Nicolas de Bonaventure for adding a rayonnant to the cathedral. Ten years later,
Jean Mignot arrived from Paris to improve upon the work done. After examining
the building, the French engineer decided that it was in "pericolo di ruina" and
that the project had been done "sine scienzia". I never understood why a French
engineer would speak in the Tuscan dialect, instead of French, but I think that
this is not the focus of the discussion. Magot's forecasts spurred Galeazzo to
improve his technique, thus the peril of ruin was eliminated. Gian Galeazzo died
in 1402, and the construction stalled for 80 years due to lack of ideas. In
1488, Leonardo da Vinci and Donato Bramante competed for designing the central
cupola, which was completed between 1500 and 1510. Around 1638, Francesco Richini
constructed five portals and the middle windows. In 1682, the roof covering was
completed. In 1762, the Madonnina's spire was erected at the height of 108 m.
In 1805, Napoleon Bonaparte ordered the façade to be finished by Pellicani.

If it is so hard to build a cathedral, one should not expect that Aritificial
Intelligence will be ready to deploy in the next decade or so. Therefore, researchers
and engineers need a language like Lisp, which demonstrated that it can last
at least a few decades, let us hope that it can last a few centuries. I used to
program in Pascal. A couple of months ago, I tried to download Free Pascal, a poorly
maintained version of the language. To my dismay, Free Pascal does not work anymore
in the Macintosh. However, I keep using Lisp programs created 30 years ago, and
they run in computers that even did not exist when the software was designed:
Maxima, ACL2, Screamer, Elephant, Frames, etc. Why Lisp is so resilient?

Computer languages have, in general, a very complex syntax. A  Python programmer,
for instance, must learn many syntactical variants for calling up an expression.
In the snippet below, the function that calculates the logarithm has a prefix notation,
while arithmetic operations obey infix syntax rules.

```python

>>> math.log(3,4)
0.7924812503605781
>>> (3+4)*(5+6+7)*8
1008

```

A few years ago, I learned Python and wrote a few programs for converting
between ebook formats, and creating dictionaries. Then, I tried to run
the programs last week. They did not work. I remembered that I made a Haskell
version, and I tried to compile it. It did not compile. Finally, I found a
version in Bigloo, a dialect of Scheme. It compiled withou a flinch. Why?

Instead of accepting an arbitrary syntax, Lisp  adopted symbolic expressions,
which evolved from a mathematical notation proposed by Lukasiewicz in 1920.
In mathematics, basic constructions and transformation rules are kept to a minimum.
This *lex parsimoniae* has many consequences. The first being that few rules can
classify all symbolic expressions:

1. Quoted lists such as `'(a b e)` represent sequences of objects.
2. Unquoted  lists such as `(log 8 2)` denote function calls.
3. Abstractions such as `(lambda(x y) (log (abs x) y))`  define   functions. The
   sequence of symbols `(x y)` is called a binding list, and the expression
   `(log (abs x) y)` is the body of the abstraction.

Any variable that appears in the binding list is said to be bound. Free variables
occur in the body of the abstraction but not in the binding list.

Another consequence of its mathematical foundations is that Lisp has computation rules
based on a Mathematical system called the /lambda$-Calculus/.

1. alpha-conversion -- Changing the name of a bound variable produces equivalent expressions.
   Therefore,  `(lambda(x) x)` and `(lambda(y) y)` are equivalent.
2. beta-reduction -- `((lambda(x) E) v)` can be reduced to `E[with x replaced by v]`.

Lisp has a Read Eval Print Loop (REPL) that performs an acceptable imitation of `beta-reduction`
over symbolic expressions in order to simplify their forms. In the `beta-reduction` process,
the first element of a list can be considered as  a function or a macro, and the other
elements are considered arguments:

```
> (log 3 4)
0.7924812503605781
> (* (+ 3 4) (+ 5 6) 8)
616
> (* (+ 3 4) (+ 5 6 7) 8)
1008
```
One immediate advantage of its mathematical foundations is that Lisp is unlikely to become
obsolete. This means that one may expect that a language like Python or Fortran suffer
changes without backward compatibility. One can even expect that Python or Fortran would
be phased out. However, Lisp code written many decades ago can be easily run in a modern
computer. Besides this, since Lisp does not change, computer scientists can work on Lisp
compilers and applications for many decades, which results in accumulation of fast and
robust code.

My confidence that Lisp can be used to build cathedrals stems from its mathematical
foundations. The principles of a mathematical theory are small in number and orthogonal,
which means that you cannot obtain one axiom from the others. A language with a rich
and arbitrary syntax, such as Haskell, is a continuous temptation for improvers.
However, Lisp has the mathematical requirement that its syntax must be the smaller
ever. Since Lisp syntax is very close to the minimum, there is no space for manoeuvre.

To avoid obsolescence, my philosophy of programming can be resumed into the following rules:

1. Use only functional application, definition, cond form, set! and let.
2. You can use any primitive functior you choose, car, cdr, cons, log, etc.
   If you think that a function may change name, place a comment explaining
   what it does, so people who need your program in the year 3059, may
   define it in a quantum computer
3. You may define macros. Use defmacro in Common Lisp, syntax-case in Scheme.
   This choice aims at minimizing the difference between Common Lisp macros
   and Scheme macros.
4. Don't design macros with more than 50 lines.

There is one thing more to discuss. The language C has a very complex syntax
and seems to have survived more or less intact. How this fit in the above
theory? There is another way for resisting obsolescence, besides mathematical
minimization: When people involved in a long term project maintain the
computer language used in the project, they may ban changes. For instance,
GNU-Linux is a long term project. The same person who created GNU-Linux,
Richard Stallman, maintains gcc, and does not accept changes that he himself
approved. However, competition is appearing. I have many programs written
for gcc that does not run in clang.

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










