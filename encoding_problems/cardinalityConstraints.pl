% Cardinality constraints
:-dynamic(auxId/1).

exactlyZero([X|L]):-
	% All of them need to be false.
	writeClause([ \+X ]),
	exactlyZero(L).
exactlyZero([]).

exactlyOne(A):-
	atLeastOne(A),
	atMostOne(A).

exactlyTwo(A):-
	atMostTwo(A),
	atLeastTwo(A).
	
exactlyThree(A):-
	atMostThree(A),
	atLeastThree(A).
	
% At Least One
atLeastOne(A):-
	writeClause(A).

% At Most One
auxId(0).
log2(N, X):- X is ceil(log(N) / log(2)).
negate(\+X, X).
negate(X, \+X).
atMostOne([]).
atMostOne(L):-
	heuleAMO(L),
	retract( auxId(A) ),
	A1 is A + 1,
	assert( auxId(A1) ), !.
	
heuleAMO(L):-
	heuleAMO(L, 0).
	
heuleAMO([V1, V2, V3, V4, V5, V6 | L], D):-
	auxId(A),
	Aux = heule-A-D,
	simpleAMO([V1, V2, V3, V4, Aux]),
	append(L, [V5, V6, \+Aux], L1),
	D1 is D + 1,
	heuleAMO(L1, D1).

heuleAMO(L, _):-
	simpleAMO(L).

heuleAMO([_], _).
heuleAMO([], _).

simpleAMO([]).
simpleAMO([V1 | L]):-
	negate(V1, V1n),
	simpleAMO(V1n, L),
	simpleAMO(L).

simpleAMO(_, []).
simpleAMO(V1n, [V2|L]):-
	negate(V2, V2n),
	writeClause([ V1n, V2n ]),
	simpleAMO(V1n, L).

% At Most Two
atMostTwo([V1, V2, V3, V4]):-
	% Obtained translating a BDD to CNF
	writeClause([ \+V1, \+V2, \+V3 ]),
	writeClause([ \+V1, V2, \+V3, \+V4]),
	writeClause([ \+V1, \+V2, V3, \+V4 ]),
	writeClause([ V1, \+V2, \+V3, \+V4 ]).

atMostTwo([V1, V2, V3]):-
	writeClause([ \+V1, \+V2, \+V3 ]).

atMostTwo([_, _]).

% At Least Two
atLeastTwo([V1, V2, V3, V4]):-
	% Obtained translating a BDD to CNF
	writeClause([ V1, V2, V3 ]),
	writeClause([ \+V1, V2, V3, V4 ]),
	writeClause([ V1, \+V2, V3, V4 ]),
	writeClause([ V1, V2, \+V3, V4 ]).
	
% At Least Three
atLeastThree([V1, V2, V3, V4]):-
	% Obtained translating a BDD to CNF
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
