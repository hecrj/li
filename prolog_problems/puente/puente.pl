:-include('../solver.pl').

solve:- puente.

puente:- solucionOptima(puente-i-[1,2,5,8]-[0,0,0,0], puente-d-[0,0,0,0]-[1,2,5,8]).

coste([], 0).
coste([puente-_-_-_], 0).
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
	cruzar2(SinCruzar1, Cruzado1, SinCruzar2, Cruzado2).

unPaso(puente-d-SinCruzar1-Cruzado1, puente-i-SinCruzar2-Cruzado2):-
	cruzar1(Cruzado1, SinCruzar1, Cruzado2, SinCruzar2).

cruzar2(SinCruzar1, Cruzado1, SinCruzar2, Cruzado2):-
	member(X, SinCruzar1), X \= 0,
	member(Y, SinCruzar1), Y \= 0,
	X \= Y,
	cruza(X, SinCruzar1, Cruzado1, SinCruzar, Cruzado),
	cruza(Y, SinCruzar, Cruzado, SinCruzar2, Cruzado2).

cruzar1(SinCruzar1, Cruzado1, SinCruzar2, Cruzado2):-
	member(X, SinCruzar1), X \= 0,
	cruza(X, SinCruzar1, Cruzado1, SinCruzar2, Cruzado2).

cruza(1, [1, S1, S2, S3], [0, C1, C2, C3], [0, S1, S2, S3], [1, C1, C2, C3]).
cruza(2, [S1, 2, S2, S3], [C1, 0, C2, C3], [S1, 0, S2, S3], [C1, 2, C2, C3]).
cruza(5, [S1, S2, 5, S3], [C1, C2, 0, C3], [S1, S2, 0, S3], [C1, C2, 5, C3]).
cruza(8, [S1, S2, S3, 8], [C1, C2, C3, 0], [S1, S2, S3, 0], [C1, C2, C3, 8]).
