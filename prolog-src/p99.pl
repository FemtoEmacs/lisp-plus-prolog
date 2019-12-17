%% P01 (*): Find the last element of a list
%% > ?-mylast(X, [1,2,3,4,5]).
%% X = 5

mylast(X, [X]) :- !.
mylast(X, [_|L]) :- mylast(X, L).

%% P02 (*): Find the last but one element of a list
%% > ?-last_but_one(X, [1,2,3,4,5]).
%% X = 4

last_but_one(X, [X,_]) :- !.
last_but_one(X, [_,Y|Ys]) :- last_but_one(X, [Y|Ys]).

% P03 (*): Find the K'th element of a list.
% The first element in the list is number 1.
%% > ?-element_at(X, [1,2,three,4,5], 3).
%% X = three
element_at(X,[X|_],1) :- !.
element_at(X,[_|L],K) :- K > 1,
   K1 is K - 1, element_at(X,L,K1).

% P04 (*): Find the number of elements of a list.
%% > ?-length([1,2,3,4,5], L).
%% L = 5

length([],0) :- !.
length([_|L],N) :- length(L,N1),
                  N is N1 + 1.

% P05 (*): Reverse a list.
%% > ?-reverse([1,2,3,4],L).
%% L = [4,3,2,1]

reverse(L1,L2) :- my_rev(L1,L2,[]).

my_rev([],L2,L2) :- !.
my_rev([X|Xs],L2,Acc) :- my_rev(Xs,L2,[X|Acc]).

% P06 (*): Find out whether a list is a palindrome
%% > ?-is_palindrome([s,u,b,i,d,u,r,a,a,r,u,d,i,b,u,s]).
%% yes

%% Ignore spaces in tests for palindrome. Palindromes
%% in Latin (Roman did not use spaces):
%  subi dura a rudibus -- endure rudeness from peasants
%  ablata at alba -- secluded but pure
%  roma tibi subito motibus ibit amor -- 
%    in Rome quickly with its bustle you will find love
%  in girum imus nocte et consumimur igni --
%     we go about in the night and are consumed by fire
  
is_palindrome(L) :- reverse(L,L).


% P07 (**): Flatten a nested list structure.
%% > ?-flatten([3,4,[a,[b]], 5], L).
%% L = [3,4,a,b,5]

append([],L,L) :- !.
append([H|T], L, [H|U]) :- append(T,L,U).

flatten([],[]) :- !.
flatten([X|Xs],Zs) :- !, flatten(X,Y),
   flatten(Xs,Ys), append(Y,Ys,Zs).
flatten(X, [X]).

% P08 (**): Eliminate consecutive duplicates of list elements.
%% > ?-compress([2,3,3,3,4,4,5], L).
%% L = [2,3,4,5]
compress([],[]) :- !.
compress([X],[X]) :- !.
compress([X,Y|Xs],Zs) :-  X==Y, !, compress([X|Xs],Zs).
compress([X,Y|Ys],[X|Zs]) :-  compress([Y|Ys],Zs).


% P09 (**):  Pack consecutive duplicates of list elements.
%% > ?-pack([2,3,3,3,4,4,5], L).
%% L = [[2],[3,3,3],[4,4],[5]]

pack([],[]) :- !.
pack([X|Xs],[Z|Zs]) :- transfer(X,Xs,Ys,Z), pack(Ys,Zs).

% transfer(X,Xs,Ys,Z) Ys is the list that remains from the list Xs
%    when all leading copies of X are removed and transfered to Z

transfer(X,[],[],[X]) :- !.
transfer(Y,[X|Xs],Ys,[X|Zs]) :- X == Y, !, transfer(X,Xs,Ys,Zs).
transfer(X,[Y|Ys],[Y|Ys],[X]).


% P10 (*):  Run-length encoding of a list
%% > ?-encode([a,a,a,b,b,b,b,b,c,d], Ans).
%% Ans = [[3,a],[5,b],[1,c],[1,d]]

encode(L1,L2) :- pack(L1,L), transform(L,L2).

transform([],[]) :- !.
transform([[X|Xs]|Ys],[[N,X]|Zs]) :- length([X|Xs],N),
                             transform(Ys,Zs).

% P11 (*):  Modified run-length encoding
%% > ?-encode_modified([1,1,2,2,2,2,2,3,3,4], G).
%% G = [[2,1],[5,2],[2,3],4]

encode_modified(L1,L2) :- encode(L1,L), strip(L,L2).

strip([],[]) :- !.
strip([[Num,X]|Ys],[X|Zs]) :- Num == 1, !, strip(Ys,Zs).
strip([[N,X]|Ys],[[N,X]|Zs]) :- N > 1, strip(Ys,Zs).


% P12 (**): Decode a run-length compressed list.
%% > ?-encode([a,a,a,b,b,b,b,b,c,d], Ans), decode(Ans, G).
%% Ans = [[3,a],[5,b],[1,c],[1,d]]
%%G = [a,a,a,b,b,b,b,b,c,d]

notlist(X) :- atom(X), !.
notlist(X) :- number(X).

decode([],[]) :- !.
decode([[Num,X]|Ys],[X|Zs]) :- Num == 1, !, decode(Ys,Zs).
decode([[N,X]|Ys],[X|Zs]) :- N > 1, !, N1 is N - 1,
           decode([[N1,X]|Ys],Zs).
decode([X|Ys],[X|Zs]) :- notlist(X),  decode(Ys,Zs).


% P13 (**): Run-length encoding of a list (direct solution) 
%% > ?-encode_direct([1,1,2,2,2,2,2,3,3,4], G).
%% G = [[2,1],[5,2],[2,3],4]

encode_direct([],[]) :- !.
encode_direct([X|Xs],[Z|Zs]) :- count(X,Xs,Ys,1,Z),
                     encode_direct(Ys,Zs).

count(X,[],[],1,X) :- !.
count(X,[],[],N,[N,X]) :- N > 1, !.
count(X,[Y|Ys],[Y|Ys],1,X) :- X \= Y.
count(Y,[X|Xs],Ys,K,T) :- X == Y, !,
   K1 is K + 1, count(X,Xs,Ys,K1,T).
count(X,[Y|Ys],[Y|Ys],N,[N,X]) :- N > 1.

% P14 (*): Duplicate the elements of a list
%% > ?-dupli([1,2,2,3,3,3,5], L).
%% L = [1,1,2,2,2,2,3,3,3,3,3,3,5,5]

dupli([],[]) :- !.
dupli([X|Xs],[X,X|Ys]) :- dupli(Xs,Ys).

% P15 (**): Duplicate the elements of a list agiven number of times
%% > ?-dupli([1,2,2,3,3,3,5], 4,  L).
%% L = [1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,5,5,5,5]

dupli(L1,N,L2) :- dupli(L1,N,L2,N).

dupli([],_,[],_) :- !.
dupli([_|Xs],N,Ys, K) :- K == 0, !, dupli(Xs,N,Ys,N).
dupli([X|Xs],N,[X|Ys],K) :- K > 0, K1 is K - 1,
                dupli([X|Xs],N,Ys,K1).

% P16 (**):  Drop every N'th element from a list
%% > ?-drop([1,2,3,4,5,6], 3, R).
%% R = [1,2,4,5]

drop(L1,N,L2) :- drop(L1,N,L2,N).

drop([],_,[],_) :- !.
drop([X|Xs],N,[X|Ys],K) :- K > 1, !,
       K1 is K - 1, drop(Xs,N,Ys,K1).
drop([_|Xs],N,Ys, K) :-  drop(Xs,N,Ys,N).

% P17 (*): Split a list into two parts
%% > ?-split([2,5,3,8,1], 3, X, Y).
%% X = [2,5,3]
%% Y = [8,1]

split([X|Xs],N,[X|Ys],Zs) :- N > 0, !, N1 is N - 1,
                 split(Xs,N1,Ys,Zs).
split(L,0,[],L).


% P18 (**):  Extract a slice from a list
%% > ?-slice([1,2,3,4,5,6,7], 2,4, R).
%% R = [2,3,4]

slice([X|_],1,1,[X]) :- !.
slice([X|Xs],1,K,[X|Ys]) :- K > 1, !, 
   K1 is K - 1, slice(Xs,1,K1,Ys).
slice([_|Xs],I,K,Ys) :- I > 1, 
   I1 is I - 1, K1 is K - 1, slice(Xs,I1,K1,Ys).


% P19 (**): Rotate a list N places to the left 
%% > ?-rotate([1,2,3,4,5,6,7], 2, R).
%% R = [3,4,5,6,7,1,2]

rotate(L1,N,L2) :- N >= 0, !, 
   length(L1,NL1), N1 is N mod NL1,
   rotate_left(L1,N1,L2).
rotate(L1,N,L2) :- 
   length(L1,NL1), N1 is NL1 + (N mod NL1),
   rotate_left(L1,N1,L2).

rotate_left(L1,N,L2) :- N > 0, !,
    split(L1,N,S1,S2), append(S2,S1,L2).
rotate_left(L,0,L).

% P20 (*): Remove the K'th element from a list.
%% > ?-remove_at(X, [1,2,3,4,5,6], 3, L).
%% X = 3
%% L = [1,2,4,5,6]

remove_at(X,[Y|Xs],K,[Y|Ys]) :- K > 1, !, 
   K1 is K - 1, remove_at(X,Xs,K1,Ys).
remove_at(X,[X|Xs],1,Xs).


% P21 (*): Insert an element at a given position into a list
%% > ?-insert_at(a,[1,2,3,4,5], 3, L).
%% L = [1,2,a,3,4,5]

insert_at(X,L,K,R) :- remove_at(X,R,K,L).


% P22 (*):  Create a list containing all integers within a given range.
%% > ?-range(4,9,L).
%% L = [4,5,6,7,8,9]

range(I,K,[I]) :- I >= K, !.
range(I,K,[I|L]) :- I < K, 
    I1 is I + 1, range(I1,K,L).


% P23 (**): Extract a given number of randomly
% selected elements from a list.
%% (load "rnd.lisp") the Lisp function
%% that generates random numbers.
%% Study file "rnd.lisp" to learn how to
%% implement new primitives.
%% > ?- rnd_select([1,2,3,4,5,6], 3, L).
%% L = [6,2,4]

rnd_select(_,K,[]) :- K < 1, !.
rnd_select(Xs,N,[X|Zs]) :- N > 0,
    length(Xs,L),
    random(R, L), I is R + 1,
    remove_at(X,Xs,I,Ys),
    N1 is N - 1,
    rnd_select(Ys,N1,Zs).


% P24 (*): Lotto: Draw N different random numbers from  1..M
%% > ?-lotto(6,49,L).
%% L = [44,36,39,5,28,34]

lotto(N,M,L) :- range(1,M,R), rnd_select(R,N,L).


% P25 (*):  Generate a random permutation of the elements of a list
%% > ?-rnd_permu([1,2,3,4,5], L).
%% L = [2,4,3,1,5]

rnd_permu(L1,L2) :- length(L1,N), rnd_select(L1,N,L2).


% P26 (**):  Generate the combinations of k distinct objects
%            chosen from the n elements of a list.

% combination(K,L,C) :- C is a list of K distinct elements 
%    chosen from the list L
%% > ?- combination(3, [a,b,c,d], L).
%% L = [a,b,c]
%% ?;
%% L = [a,b,d]
%% ?;
%% L = [a,c,d]
%% ?;
%% L = [b,c,d]
%% ?;
%%
%% no.

combination(0,_,[]).
combination(K,L,[X|Xs]) :- K > 0,
   el(X,L,R), K1 is K-1, combination(K1,R,Xs).

% Find out what the following predicate el/3 exactly does.

el(X,[X|L],L).
el(X,[_|L],R) :- el(X,L,R).

atm(X) :- atomic(X).
atm(X) :- var(X).

var_memberchk(A0, [A1|_]) :- 
     A0 == A1, !, atm(A0).
var_memberchk(A0, [_|R]) :- 
    var_memberchk(A0, R).

subtract([], _, []).
subtract([A|C], B, D) :-
    var_memberchk(A, B), 
    subtract(C, B, D).
subtract([A|B], C, [A|D]) :-
    \+ var_memberchk(A,C),
    subtract(B, C, D).


% P27 (**) Group the elements of a set into disjoint subsets.

% Problem a)

% group3(G,G1,G2,G3) :- distribute G into G1, G2, and G3,
% so that G1, G2 and G3 have 2,3 & 4 elements respectively
%% > ?-group3([aldo, beat, carla, david,
%%              evi, flip, gary, hugo, ida], G1,G2,G3).
%% G1 = [aldo,beat]
%% G2 = [carla,david,evi]
%% G3 = [flip,gary,hugo,ida]
%% ?;
%% G1 = [aldo,beat]
%% G2 = [carla,david,flip]
%% G3 = [evi,gary,hugo,ida]
%% ?;
group3(G,G1,G2,G3) :- 
   selectN(2,G,G1),
   subtract(G,G1,R1),
   selectN(3,R1,G2),
   subtract(R1,G2,R2),
   selectN(4,R2,G3),
   subtract(R2,G3,[]).

% selectN(N,L,S) :- select N elements of list L
%    and put them in set S. Via backtracking return
%    all posssible selections, but avoid permutations;
%    i.e. after generating S = [a,b,c] do not return
%    S = [b,a,c], etc.

selectN(0,_,[]) :- !.
selectN(N,L,[X|S]) :- N > 0, 
   el(X,L,R), 
   N1 is N-1,
   selectN(N1,R,S).

% Problem b): Generalization

% group(G,Ns,Gs) :- distribute G into the groups Gs.
%    The group sizes are given in the list Ns.
%% > ?- group([aldo,beat,carla,david,evi,
%%             flip,gary,hugo,ida],[2,2,5],Gs).
%% Gs = [[aldo,beat],[carla,david],[evi,flip,gary,hugo,ida]]
%% ?;
%% Gs = [[aldo,beat],[carla,evi],[david,flip,gary,hugo,ida]]
%% ?y

group([],[],[]).
group(G,[N1|Ns],[G1|Gs]) :- 
   selectN(N1,G,G1),
   subtract(G,G1,R),
   group(R,Ns,Gs).


% P28 (**) Determine whether a given integer number is prime. 

% is_prime(P) :- P is a prime number
%    (integer) (+)

is_prime(2).
is_prime(3).
is_prime(P) :- integer(P), P > 3, P mod 2 =\= 0, \+ has_factor(P,3).  

% has_factor(N,L) :- N has an odd factor F >= L.
%    (integer, integer) (+,+)

has_factor(N,L) :- N mod L =:= 0.
has_factor(N,L) :- L * L < N, L2 is L + 2, has_factor(N,L2).

% P29 (**) Determine the greatest common divisor of two positive integers.
%% > ?- gcd(36,63, F).
%% F = 9
%% ?;

gcd(X,0,X) :- X > 0.
gcd(X,Y,G) :- Y > 0, Z is X mod Y, gcd(Y,Z,G).


% P30 (*) Determine whether two positive integer
% numbers are coprime. Two numbers are coprime if
% their greatest common divisor equals 1.
%% > ?- coprime(35,64).
%% yes.

coprime(X,Y) :- gcd(X,Y,1).



