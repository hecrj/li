nat(0).
nat(N):- nat(X), N is X + 1.

camino(_, E,E, C,C ).
camino(N, EstadoActual, EstadoFinal, CaminoHastaAhora, CaminoTotal ):-
	podar(CaminoHastaAhora, N),
	unPaso( EstadoActual, EstSiguiente ),
	\+contiene(EstSiguiente, CaminoHastaAhora),
	camino(N, EstSiguiente, EstadoFinal, [EstSiguiente|CaminoHastaAhora], CaminoTotal ).

% Solo podamos si el problema lo requiere.
podar(_, _):- podar(0), !.
podar(CaminoHastaAhora, N):- coste(CaminoHastaAhora, C), C =< N.

solucionOptima(I, F):-
	nat(N),
	camino(N, I, F, [I], C),
	coste(C, N),
	reverse(C, R),
	write('Optimal cost: '), write(N), nl,
	write('Steps:'), nl,
	display(R).

display([]).
display([Step|L]):- write('    '), write(Step), nl, display(L).

main:- solve, halt.
