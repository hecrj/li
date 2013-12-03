nat(0).
nat(N):- nat(X), N is X + 1.

camino(_, E,E, C,C ).
camino(N, EstadoActual, EstadoFinal, CaminoHastaAhora, CaminoTotal ):-
	coste(CaminoHastaAhora, C), C =< N,
	unPaso( EstadoActual, EstSiguiente ),
	\+member(EstSiguiente, CaminoHastaAhora),
	%write(CaminoHastaAhora), write(C), nl,
	camino(N, EstSiguiente, EstadoFinal, [EstSiguiente|CaminoHastaAhora], CaminoTotal ).

solucionOptima(I, F):-
	nat(N), % Buscamos solucion de "coste" 0; si no, de 1, etc.
	write(N), nl,
	camino(N, I, F, [I], C),
	coste(C, N),
	reverse(C, R),
	write('Optimal cost: '), write(N), nl,
	write('Steps:'), nl,
	display(R).

display([]).
display([Step|L]):- write('    '), write(Step), nl, display(L).

main:- solve, halt.
