selecciones([
    [1,2,6], [1,6,7], [2,3,8], [6,7,9], [6,8,9], [1,2,4], [3,5,6], [3,5,7],
    [5,6,8], [1,6,8], [4,7,9], [4,6,9], [1,4,6], [3,6,9], [2,3,5], [1,4,5],
    [1,6,7], [6,7,8], [1,2,4], [1,5,7], [2,5,6], [2,3,5], [5,7,9], [1,6,8]
]).

slots([a, b, c]).

main:- solve, halt.

nat(0).
nat(N):- nat(X), N is X + 1.

solve:-
    nat(N).

asignar(Seleccion, Asignacion):-
    asignar(Seleccion, [], Asignacion).

asignar([], [], []).
asignar([_ | L1], L2, [A | L3]):-
    slots(S),
    member(A, S),
    \+member(A, L2),
    asignar(L1, [A | L2], L3).

