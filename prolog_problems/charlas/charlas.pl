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
    inicial(Inicial),
    write('Computando...'), nl,
    selecciones(Selecciones),
    nat(N),
    solve(Inicial, Selecciones, N, Solucion),
    coste(Solucion, Coste),
    Coste =< N,
    display(Solucion).

solve(Actual, [], _, Actual).
solve(Actual, [Charlas | RestoCharlas], N, Solucion):-
    coste(Actual, Coste),
    Coste =< N,
    asignar(Charlas, Slots),
    actualizar(Actual, Charlas, Slots, Siguiente),
    solve(Siguiente, RestoCharlas, N, Solucion).

display(Asignaciones-Coste):-
    write('Solución:'), nl,
    write('    Hay que dar '), write(Coste), write(' charlas.'), nl,
    write('    Asignaciones:'), nl,
    display(Asignaciones),
    write('----------------------------------------'), nl,
    write('        En todos: '),
    enTodoSlot(Asignaciones, CharlasEnTodoSlot),
    sort(CharlasEnTodoSlot, CharlasEnTodoSlotOrdenadas),
    write(CharlasEnTodoSlotOrdenadas), nl.

display([]).
display([ (Slot, Charlas) | RestoAsignaciones]):-
    write('        Slot '), write(Slot), write(':   '),
    sort(Charlas, CharlasOrdenadas),
    write(CharlasOrdenadas), nl,
    display(RestoAsignaciones).

enTodoSlot([ (_, Charlas) | RestoAsignaciones ], CharlasEnTodoSlot):-
    enTodoSlot(RestoAsignaciones, Charlas, CharlasEnTodoSlot).

enTodoSlot([], CharlasHastaAhora, CharlasHastaAhora).
enTodoSlot([ (_, Charlas) | RestoAsignaciones], CharlasHastaAhora, CharlasEnTodoSlot):-
    intersection(Charlas, CharlasHastaAhora, CharlasHastaAhora1),
    enTodoSlot(RestoAsignaciones, CharlasHastaAhora1, CharlasEnTodoSlot).

inicial(Inicial):-
    slots(Slots),
    inicial(Asignaciones, Slots),
    Inicial = Asignaciones-0.

inicial([], []).
inicial([ (Slot, []) | RestoAsignaciones ], [ Slot | RestoSlots ]):- inicial(RestoAsignaciones, RestoSlots).

coste(_-Coste, Coste).

asignar(Charlas, Slots):-
    asignar(Charlas, [], Slots).

asignar([], _, []).
asignar([_ | RestoCharlas], SlotsAsignados, [Slot | RestoSlots]):-
    slots(Slots),
    member(Slot, Slots),
    \+member(Slot, SlotsAsignados),
    asignar(RestoCharlas, [Slot | SlotsAsignados], RestoSlots).

actualizar(Asignaciones-Coste, [], [], Asignaciones-Coste).
actualizar(Asignaciones-Coste, [ Charla | RestoCharlas ], [ Slot | RestoSlots ], Siguiente-NuevoCoste):-
    actualizar(Asignaciones, Coste, Charla, Slot, SiguienteParcial, NuevoCosteParcial),
    actualizar(SiguienteParcial-NuevoCosteParcial, RestoCharlas, RestoSlots, Siguiente-NuevoCoste).

actualizar([], Coste, _, _, [], Coste).
actualizar([ Asignacion | RestoAsignaciones ], Coste, Charla, Slot, [ AsignacionNueva | RestoSiguiente ], NuevoCoste):-
    programar(Asignacion, Coste, Charla, Slot, AsignacionNueva, CosteParcial),
    actualizar(RestoAsignaciones, CosteParcial, Charla, Slot, RestoSiguiente, NuevoCoste).

programar((Slot1, Charlas), Coste, _, Slot2, (Slot1, Charlas), Coste):-
    Slot1 \= Slot2.

programar((Slot, Charlas), Coste, Charla, Slot, (Slot, Charlas), Coste):-
    member(Charla, Charlas).

programar((Slot, Charlas), Coste, Charla, Slot, (Slot, NuevasCharlas), NuevoCoste):-
    \+member(Charla, Charlas),
    append([Charla], Charlas, NuevasCharlas),
    NuevoCoste is Coste + 1.
