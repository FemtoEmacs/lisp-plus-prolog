%% File: ?- consult('fvalue.pl')

period_of_time(2).
period_of_time(4).
period_of_time(8).
period_of_time(10).

futval(PV, I, N, FV) :- period_of_time(N),
                 FV is PV*(1+I/100)**N.
