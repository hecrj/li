:-include('../solver.pl').

solve:- puente.

puente:- solucionOptima(puente-i-[1, 2, 5, 8]-[0, 0, 0, 0], puente-d-[0, 0, 0, 0]-[1, 2, 5, 8]).

coste([], 0).
coste([puente-_-_-Cruzado], C):- max(Cruzado, C).
coste([puente-d-_-Despues, X | L], C):-
	X = puente-i-Antes-_,
	coste(Antes, Despues, [X | L], C).

coste([puente-i-Despues-_, X | L], C):-
	X = puente-d-_-Antes,
	coste(Antes, Despues, [X | L], C).

coste(Antes, Despues, L, C):-
	intersection(Antes, Despues, Cruzan),
	max(Cruzan, C1),
	coste(L, C2),
	C is C1 + C2.

max([], 0).
max([X], X).
max([X|L], X):- max(L, Y), X >= Y.
max([X|L], N):- max(L, N), N > X.

unPaso(puente-i-SinCruzar1-Cruzado1, puente-d-SinCruzar2-Cruzado2):-
	cruzar(SinCruzar1, Cruzado1, SinCruzar2, Cruzado2).

unPaso(puente-d-SinCruzar1-Cruzado1, puente-i-SinCruzar2-Cruzado2):-
	cruzar(Cruzado1, SinCruzar1, Cruzado2, SinCruzar2).

cruzar(SinCruzar1, Cruzado1, SinCruzar2, Cruzado2):-
	seleccionar(SinCruzar1, Cruzan),
	append(Cruzado1, Cruzan, Cruzado2),
	subtract(SinCruzar1, Cruzan, SinCruzar2).

seleccionar(SinCruzar, [X, Y]):-
	member(X, SinCruzar),
	member(Y, SinCruzar),
	X \= Y.
seleccionar(SinCruzar, [X]):- member(X, SinCruzar).
