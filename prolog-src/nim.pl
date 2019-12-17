%% ?-consult('nim.pl').

ap([], L, L).
ap([H|T], L, [H|U]) :- ap(T, L, U).

tk([X],V,V).
tk([X,X1|Y],V,[[X1|Y]|V]).
tk([X|T],V,Y) :- tk(T,V,Y).

mv(X,Y) :- ap(U,[X1|V],X),
       tk(X1,V,R),ap(U,R,Y).

us(X,Y) :- mv(X,Y), \+ them(Y,Z).
them(X,Y) :- mv(X,Y), \+ us(Y,Z).

