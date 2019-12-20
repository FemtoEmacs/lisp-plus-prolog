%% (load "prolog.scm")
%% (logic "-f prolog-src/fvalue.pl")

choose(N, [N|Rest]).
choose(N, [_|Rest]) :- choose(N, Rest).

futval(PV, I, N, FV) :- choose(N, [2, 4, 8]),
                 FV is PV*(1+I/100)**N.


