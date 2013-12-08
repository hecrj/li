:-include('../solver.pl').
podar(1). % Habilitar la poda al buscar solución

solve:- puente([1, 2, 5, 8]).
capacidadPuente(2).

puente(P):-
	ceros(P, Z),
	solucionOptima(puente-i-P-Z-0, puente-d-Z-P-_).

ceros([], []).
ceros([_ | P], [0 | Z]):- ceros(P, Z).

coste([], 0).
coste([puente-_-_-_-C], C).
coste([puente-_-_-_-C | _], C).

contiene(puente-Lado-SinCruzar-Cruzado-_, [ puente-Lado-SinCruzar-Cruzado-_ | _ ]).
contiene(X, [ _ | Resto ]):- contiene(X, Resto).

unPaso(puente-i-SinCruzar1-Cruzado1-C1, puente-d-SinCruzar2-Cruzado2-C2):-
	cruzar(SinCruzar1, Cruzado1, C1, SinCruzar2, Cruzado2, C2).

unPaso(puente-d-SinCruzar1-Cruzado1-C1, puente-i-SinCruzar2-Cruzado2-C2):-
	cruzar(Cruzado1, SinCruzar1, C1, Cruzado2, SinCruzar2, C2).

cruzar(SinCruzar1, Cruzado1, C1, SinCruzar2, Cruzado2, C2):-
	capacidadPuente(Max),
	seleccionar(Max, SinCruzar1, Cruzan),
	max(Cruzan, CosteCruce),
	cruzar(Cruzan, SinCruzar1, Cruzado1, SinCruzar2, Cruzado2),
	C2 is C1 + CosteCruce.

seleccionar(Max, SinCruzar, Cruzan):-
	seleccionar(Max, SinCruzar, [], Cruzan).

seleccionar(0, _, _, []):- !.
seleccionar(Max, L, Seleccionados, [ X | PorSeleccionar ]):-
	member(X, L), X \= 0,
	\+member(X, Seleccionados),
	Max1 is Max - 1,
	seleccionar(Max1, L, [ X | Seleccionados ], PorSeleccionar).

seleccionar(Max, L, Seleccionados, PorSeleccionar):-
	Max1 is Max - 1,
	seleccionar(Max1, L, Seleccionados, PorSeleccionar).

cruzar([], SinCruzar, Cruzado, SinCruzar, Cruzado).
cruzar([ Persona | Resto ], SinCruzar1, Cruzado1, SinCruzar2, Cruzado2):-
	cruza(Persona, SinCruzar1, Cruzado1, SinCruzarParcial, CruzadoParcial),
	cruzar(Resto, SinCruzarParcial, CruzadoParcial, SinCruzar2, Cruzado2).

cruza(X, [X | S1], [0 | C1], [0 | S1], [X | C1]).
cruza(X, [Y | S1], [Z | C1], [Y | S2], [Z | C2]):- cruza(X, S1, C1, S2, C2).

max([ X ], X):- !.
max([ X | Resto ], X):- max(Resto, Y), X >= Y.
max([ X | Resto ], N):- max(Resto, N), N > X.
