:-include('../solver.pl').
podar(0).

capacidad(5, 8).

solve:- cubos.
cubos:- solucionOptima(cubos-0-0, cubos-0-4).

coste([], 0).
coste([cubos-_-_|L], N):- coste(L, C), N is C + 1.

contiene(Estado, Camino):- member(Estado, Camino).

unPaso(cubos-X-_, cubos-X-0).
unPaso(cubos-_-X, cubos-0-X).
unPaso(cubos-X-_, cubos-X-C):- capacidad(_, C).
unPaso(cubos-_-X, cubos-C-X):- capacidad(C, _).
unPaso(cubos-X1-X2, cubos-Y1-Y2):-
	capacidad(_, C),
	Y2 is min(X1 + X2, C),
	Y1 is max(X1 - (C - X2), 0).
unPaso(cubos-X1-X2, cubos-Y1-Y2):-
	capacidad(C, _),
	Y1 is min(X1 + X2, C),
	Y2 is max(X2 - (C - X1), 0).
