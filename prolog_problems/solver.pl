nat(0).
nat(N):- nat(X), N is X + 1.

camino( E,E, C,C ).
camino( EstadoActual, EstadoFinal, CaminoHastaAhora, CaminoTotal ):-
	unPaso( EstadoActual, EstSiguiente ),
	\+member(EstSiguiente, CaminoHastaAhora),
	%write(EstSiguiente), nl,
	%write(CaminoHastaAhora), nl,
	camino( EstSiguiente, EstadoFinal, [EstSiguiente|CaminoHastaAhora], CaminoTotal ).

solucionOptima(I, F):-
	nat(N), % Buscamos solucion de "coste" 0; si no, de 1, etc.
	%write(N), nl,
	camino(I, F, [I], C),
	coste(C, N),
	reverse(C, R),
	write('Optimal cost: '), write(N), nl,
	write('Steps:'), nl,
	display(R).

display([]).
display([Step|L]):- write('    '), write(Step), nl, display(L).

main:- solve, halt.
