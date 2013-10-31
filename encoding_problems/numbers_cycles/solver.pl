:-include('test/entradaRodear6').
:-include('../writeClauses').
:-include(displayRodear).
:-dynamic(varNumber/3).
symbolicOutput(0).

writeClauses:- cycles, kEdges.

adjacentsToVertex(I, J, A):-
	I1 is I - 1,
	J1 is J - 1,
	rows(R), columns(C),
	(I  > 1 -> append([], [v-I1-J], L1) ; L1 = []),
	(I1 < R -> append(L1, [v-I-J],  L2) ; L2 = L1),
	(J  > 1 -> append(L2, [h-I-J1], L3) ; L3 = L2),
	(J1 < C -> append(L3, [h-I-J],  A)  ; A  = L3).

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
	(N = 0 -> exactlyZero(A) ; true),
	(N = 1 -> exactlyOne(A) ; true),
	(N = 2 -> exactlyTwo(A) ; true),
	(N = 3 -> exactlyThree(A) ; true),
	fail.
kEdges.

exactlyZero([V1, V2, V3, V4]):-
	% All of them need to be false.
	writeClause([ \+V1 ]),
	writeClause([ \+V2 ]),
	writeClause([ \+V3 ]),
	writeClause([ \+V4 ]).

exactlyOne(A):-
	writeClause(A),
	atMostOne(A).

exactlyTwo(A):-
	atMostTwo(A),
	atLeastTwo(A).
	
exactlyThree(A):-
	atMostThree(A),
	atLeastThree(A).

% At Most One
atMostOne([X|L]):-
	atMostOne(X, L),
	atMostOne(L).
atMostOne([]).

atMostOne(V, [X|L]):-
	writeClause([ \+V, \+X]),
	atMostOne(V, L).
atMostOne(_, []).

% At Most Two
atMostTwo([V1, V2, V3, V4]):-
	% BDD to CNF
	writeClause([ \+V1, \+V2, \+V3 ]),
	writeClause([ \+V1, V2, \+V3, \+V4]),
	writeClause([ \+V1, \+V2, V3, \+V4 ]),
	writeClause([ V1, \+V2, \+V3, \+V4 ]).

atMostTwo([V1, V2, V3]):-
	writeClause([ \+V1, \+V2, \+V3 ]).

atMostTwo([_, _]).

% At Least Two
atLeastTwo([V1, V2, V3, V4]):-
	% BDD to CNF
	writeClause([ V1, V2, V3 ]),
	writeClause([ \+V1, V2, V3, V4 ]),
	writeClause([ V1, \+V2, V3, V4 ]),
	writeClause([ V1, V2, \+V3, V4 ]).
	
% At Least Three
atLeastThree([V1, V2, V3, V4]):-
	% BDD to CNF
	writeClause([ V1, V2 ]),
	writeClause([ V1, \+V2, V3 ]),
	writeClause([ V1, \+V2, \+V3, V4 ]),
	writeClause([ \+V1, V2, V3 ]),
	writeClause([ \+V1, \+V2, V3, V4 ]),
	writeClause([ \+V1, V2, \+V3, V4 ]).
	
% At Most Three
atMostThree([V1, V2, V3, V4]):-
	% One of them needs to be false
	writeClause([ \+V1, \+V2, \+V3, \+V4 ]).

