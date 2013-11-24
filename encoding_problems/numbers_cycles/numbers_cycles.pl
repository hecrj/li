:-include('test/entradaRodear6').
:-include('displayRodear').
:-include('../writeClauses').
:-include('../cardinalityConstraints').
symbolicOutput(0).
useMySolver(0). % Set this to 1 to use my sat solver

writeClauses:- cycles, kEdges.

up(1, _, []):- !.
up(I, J, [v-I1-J]):- I1 is I-1.

left(_, 1, []):- !.
left(I, J, [h-I-J1]):- J1 is J-1.

down(I, _, []):- rows(R), I1 is I-1, I1 >= R, !.
down(I, J, [v-I-J]).

right(_, J, []):- columns(C), J1 is J-1, J1 >= C, !.
right(I, J, [h-I-J]).

% Relation between a vertex (I, J) and its adjacent edges
adjacentsToVertex(I, J, A):-
	up(I, J, U),
	down(I, J, D),
	left(I, J, L),
	right(I, J, R),
	
	append(U, D, L1),
	append(L1, L, L2),
	append(L2, R, A).

% Relation between a cell (I, J) and its adjacent edges
adjacentsToCell(I, J, [ h-I-J, v-I-J, h-I1-J, v-I-J1 ]):-
	I1 is I+1,
	J1 is J+1.

% Cycles constraint
% 1. Two different cycles can not share any vertices, thus
%    at most two of the adjacent edges to every vertex can be set.
% 2. To force cycles, if one adjacent edge of a vertex is set, then
%    another edge of the vertex must be set.
cycles:-
	rows(R), columns(C),
	R1 is R+1,
	C1 is C+1,
	between(1, R1, I), between(1, C1, J),
	adjacentsToVertex(I, J, A),
	atMostTwo(A),
	forceCycle(A),
	fail.
cycles.

% Forces cycles
% - V1 -> (V2 v V3 v ... v Vn)
% - ...
% - Vn -> (V1 v V2 v ... v Vn-1)
forceCycle([C|L]):-
	forceCycle([], C, L).

forceCycle(L1, C, [R|L2]):-
	negate(C, N),
	append(L1, L2, L),
	writeClause([N,R|L]),
	forceCycle([C|L1], R, L2).
	
forceCycle(L, C, []):-
	negate(C, N),
	writeClause([N|L]).

% If a cell (I, J) has a number N, then N adjacent edges
% of that cell must be set.
kEdges:-
	rows(R), columns(C),
	between(1, R, I), between(1, C, J),
	num(I, J, N),
	adjacentsToCell(I, J, A),
	kEdges(N, A),
	fail.
kEdges.

kEdges(0, A):- exactlyZero(A).
kEdges(1, A):- exactlyOne(A).
kEdges(2, A):- exactlyTwo(A).
kEdges(3, A):- exactlyThree(A).
