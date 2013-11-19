:-include('test/entradaRodear9').
:-include('displayRodear').
:-include('../writeClauses').
:-include('../cardinalityConstraints').
:-dynamic(varNumber/3).
symbolicOutput(0).

writeClauses:- cycles, kEdges.

up(1, _, []).
up(I, J, [v-I1-J]):- I1 is I-1.

left(_, 1, []).
left(I, J, [h-I-J1]):- J1 is J-1.

down(I, _, []):- rows(R), I1 is I-1, I1 >= R.
down(I, J, [v-I-J]).

right(_, J, []):- columns(C), J1 is J-1, J1 >= C.
right(I, J, [h-I-J]).

adjacentsToVertex(I, J, A):-
	up(I, J, U),
	down(I, J, D),
	left(I, J, L),
	right(I, J, R),
	
	append([], U, L1),
	append(L1, D, L2),
	append(L2, L, L3),
	append(L3, R, A).

adjacents(I, J, A):-
	I1 is I+1,
	J1 is J+1,
	A = [ h-I-J, v-I-J, h-I1-J, v-I-J1 ].

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

forceCycle([V1, V2, V3, V4]):-
	% BDD to CNF
	writeClause([ \+V1, V2, V3, V4 ]),
	writeClause([ V1, \+V2, V3, V4 ]),
	writeClause([ V1, V2, \+V3, V4 ]),
	writeClause([ V1, V2, V3, \+V4 ]).

forceCycle([V1, V2, V3]):-
	% BDD to CNF
	writeClause([ \+V1, V2, V3 ]),
	writeClause([ V1, \+V2, V3 ]),
	writeClause([ V1, V2, \+V3 ]).
	
forceCycle([V1, V2]):-
	writeClause([ \+V1, V2 ]),
	writeClause([ V1, \+V2 ]).

kEdges:-
	rows(R), columns(C),
	between(1, R, I), between(1, C, J),
	num(I, J, N),
	adjacents(I, J, A),
	kEdges(N, A),
	fail.
kEdges.

kEdges(0, A):- exactlyZero(A).
kEdges(1, A):- exactlyOne(A).
kEdges(2, A):- exactlyTwo(A).
kEdges(3, A):- exactlyThree(A).
