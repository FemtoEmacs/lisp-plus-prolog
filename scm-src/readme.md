# Test Scheme
The examples in this folder were tested in
Chez Scheme. A few examples were tested in
Racket as well. Most of the examples run
unchanged in the two languages, except for
the `#lang racket` declaration in Racket.

## Different names for procedures
There are a few instances, where procedueres
have different name in Chez Scheme and Racket.
For instance, the procedure that reads a file
line by line is called `read-line` in Racket,
and `get-line` in Chez Scheme. In this case,
a commented definition allows Racket to run
a program designed for Chez Scheme. All the
racketeer must do is uncomment the indicated
lines.

## Modules
Racket enforce the module discipline strictly.
Besides this, all files are considered modules.
Therefore, one must add declaration of `provides`
to make a function visible to the user. This will
also be indicated in a few examples.

