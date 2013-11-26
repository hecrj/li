nat(0).
nat(N):- nat(X), N is X + 1.

camino( E,E, C,C ).
camino( EstadoActual, EstadoFinal, CaminoHastaAhora, CaminoTotal ):-
	unPaso( EstadoActual, EstSiguiente ),
	\+member(EstSiguiente, CaminoHastaAhora),
	camino( EstSiguiente, EstadoFinal, [EstSiguiente|CaminoHastaAhora], CaminoTotal ).

solucionOptima(I, F):-
	nat(N), % Buscamos solucion de "coste" 0; si no, de 1, etc.
	camino(I, F, [I], C),
	coste(C, N),
	reverse(C, R),
	write(R), nl,
	write(N).

coste([], 0).

cubos:- solucionOptima(cubos-0-0, cubos-0-4).

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

misioneros:- solucionOptima(mision-i-[3, 3]-[0, 0], mision-d-[0, 0]-[3, 3]).

coste([mision-_-_-_|L], N):- coste(L, C), N is C + 1.

compLado(0, _).
compLado(M, C):- M >= C.

mover(Mo1, Co1, Md1, Cd1, Mo2, Co2, Md2, Cd2):-
	between(0, 2, Mb),
	between(0, 2, Cb),
	B is Mb + Cb,
	B > 0, B < 3,

	Mo2 is Mo1 - Mb, Co2 is Co1 - Cb,
	Mo2 >= 0, Co1 >= 0,

	Md2 is Md1 + Mb,
	Cd2 is Cd1 + Cb,

	compLado(Mo2, Co2),
	compLado(Md2, Cd2).
	

unPaso(mision-i-[Mi1, Ci1]-[Md1, Cd1], mision-d-[Mi2, Ci2]-[Md2, Cd2]):-
	mover(Mi1, Ci1, Md1, Cd1, Mi2, Ci2, Md2, Cd2).

unPaso(mision-d-[Mi1, Ci1]-[Md1, Cd1], mision-i-[Mi2, Ci2]-[Md2, Cd2]):-
	mover(Md1, Cd1, Mi1, Ci1, Md2, Cd2, Mi2, Ci2).

puente:- solucionOptima(puente-[1, 2, 5, 8]-[0, 0, 0, 0], puente-[0, 0, 0, 0]-[1, 2, 5, 8]).
