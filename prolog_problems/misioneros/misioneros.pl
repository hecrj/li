:-include('../solver.pl').
podar(0).

solve:- misioneros(3, 3).

misioneros(M, C):- solucionOptima(mision-i-[M, C]-[0, 0], mision-d-[0, 0]-[M, C]).

coste(L, C):- length(L, C).
contiene(Estado, Camino):- member(Estado, Camino).

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
