:-dynamic( solucion/2 ). % Explicacion mas abajo

selecciones([
    [1,2,6], [1,6,7], [2,3,8], [6,7,9], [6,8,9], [1,2,4], [3,5,6], [3,5,7],
    [5,6,8], [1,6,8], [4,7,9], [4,6,9], [1,4,6], [3,6,9], [2,3,5], [1,4,5],
    [1,6,7], [6,7,8], [1,2,4], [1,5,7], [2,5,6], [2,3,5], [5,7,9], [1,6,8]
]).

slots([a, b, c]).

main:- solve, halt.

nat(0).
nat(N):- nat(X), N is X + 1.

% Busca e imprime la solucion mas optima.
% Si existen varias soluciones de coste optimo N, entonces
% selecciona aquella que tenga un menor numero de charlas en todos
% los slots.
solve:-
    write('Computando...'), nl,
    inicial(Inicial),
    selecciones(Selecciones),
    nat(N),
    solve(Inicial, Selecciones, N, Solucion),
    display(Solucion).

solve(NuevaSolucion, [], MaxCoste, Solucion):-
    coste(NuevaSolucion, Coste), Coste =< MaxCoste,
    enTodoSlot(NuevaSolucion, EnTodoSlotNueva),
    length(EnTodoSlotNueva, CantidadNueva),
    guardarSolucion(NuevaSolucion, CantidadNueva, Solucion).

solve(Actual, [Charlas | RestoCharlas], MaxCoste, Solucion):-
    coste(Actual, Coste),
    Coste =< MaxCoste,
    asignar(Charlas, Slots),
    actualizar(Actual, Charlas, Slots, Siguiente),
    solve(Siguiente, RestoCharlas, MaxCoste, Solucion).

% Una vez explorado todo el arbol, devolvemos la mejor solucion encontrada.
solve(_-0, _, _, Solucion):- solucion(Solucion, _).

% Los siguientes predicados son un poco complejos.
% La idea es recorrer el arbol de soluciones una sola vez y encontrar
% la solucion con menor numero de charlas en todos los slots.
% Podria evitarse el uso de asserts y retracts, pero seria necesario recorrer
% el arbol varias veces.

% Si encontramos una solucion con 0 charlas en todos los slots, ya hemos
% terminado, ya que no existen soluciones mejores.
guardarSolucion(Solucion, 0, Solucion).

% Sino, entonces actualizamos la solucion si la nueva es mejor que la anterior.
% Finalmente, fallamos y seguimos explorando el arbol.
guardarSolucion(Nueva, CantidadNueva, _):-
    solucion(Anterior, CantidadAnterior), !,
    CantidadNueva < CantidadAnterior,
    retract( solucion(Anterior, CantidadAnterior) ),
    assert( solucion(Nueva, CantidadNueva) ), fail.

% Este caso ocurre cuando se encuentra la primera solucion con alguna charla
% en todo slot.
guardarSolucion(Nueva, CantidadNueva, _):-
    assert( solucion(Nueva, CantidadNueva) ), fail.

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

enTodoSlot(Asignaciones-_, CharlasEnTodoSlot):- enTodoSlot(Asignaciones, CharlasEnTodoSlot).

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

asignar([], _, _).
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

programar((Slot, Charlas), Coste, Charla, Slot, (Slot, Charlas), Coste):-
    member(Charla, Charlas), !.

programar((Slot, Charlas), Coste, Charla, Slot, (Slot, NuevasCharlas), NuevoCoste):-
    !, % Única opción posible para slots iguales y \+member(Charla, Charlas)
    NuevasCharlas = [ Charla | Charlas ],
    NuevoCoste is Coste + 1.

programar((Slot, Charlas), Coste, _, _, (Slot, Charlas), Coste).