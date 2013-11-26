:-include('../solver.pl').

solve:- cubos.

cubos:- solucionOptima(cubos-0-0, cubos-0-4).

coste([], 0).
coste([cubos-_-_|L], N):- coste(L, C), N is C + 1.

unPaso(cubos-X-_, cubos-X-0).
unPaso(cubos-X-_, cubos-X-8).
unPaso(cubos-_-X, cubos-0-X).
unPaso(cubos-_-X, cubos-5-X).
unPaso(cubos-X1-X2, cubos-Y1-Y2):-
	Y2 is min(X1 + X2, 8),
	Y1 is max(X1 - (8 - X2), 0).
unPaso(cubos-X1-X2, cubos-Y1-Y2):-
	Y1 is min(X1 + X2, 5),
	Y2 is max(X2 - (5 - X1), 0).
