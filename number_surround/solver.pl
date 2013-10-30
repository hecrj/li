:-include(entradaRodear1).
:-include(writeClauses).
:-include(displayRodear).
:-dynamic(varNumber/3).
symbolicOutput(0).

writeClauses:- kEdges.

adjacents(I, J, A):-
	I1 is I+1,
	J1 is J+1,
	A = [ h-I-J, v-I-J, h-I1-J, v-I-J1 ].

kEdges:- rows(R), columns(C),
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
	atMostOne(X, L).
atMostOne(_, []).

% At Most Two
atMostTwo([V1, V2, V3, V4]):-
	% BDD to CNF
	writeClause([ \+V1, \+V2, \+V3 ]),
	writeClause([ \+V1, V2, \+V3, \+V4]),
	writeClause([ \+V1, \+V2, V3, \+V4 ]),
	writeClause([ V1, \+V2, \+V3, \+V4 ]).

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

