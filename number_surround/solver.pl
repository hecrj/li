:-include(test02).
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
	(N = 1 -> exactlyOneAdjacent(I, J) ; true),
	(N = 2 -> exactlyTwoAdjacents(I, J) ; true),
	fail.
kEdges.

exactlyOneAdjacent(I, J):-
	adjacents(I, J, A),
	writeClause(A),
	atMostOne(A).

exactlyTwoAdjacents(I, J):-
	adjacents(I, J, A),
	exactlyTwo(A).

atMostOne([X|L]):-
	atMostOne(X, L),
	atMostOne(L).
atMostOne([]).

atMostOne(V, [X|L]):-
	writeClause([ \+V, \+X]),
	atMostOne(X, L).
atMostOne(_, []).

exactlyTwo([V|L]):-
	exactlyTwo(V, L).
exactlyTwo([]).

exactlyTwo(V, [X|L]):-
	% Ladder encoding
	writeClause([ \+V, V-a ]),
	writeClause([ \+V-a, X-a ]),
	writeClause([ \+V-b, X-b ]),
	writeClause([ \+V-a, \+X, X-b ]),
	writeClause([ \+V-b, \+X ]),
	
	exactlyTwo(X, L).

exactlyTwo(_, []).

atMostTwo([X|L]):-
	atMostTwo(X, L).
atMostTwo([]).

atMostTwo(V1, [V2|L]):-
	atMostTwo(V1, V2, L),
	atMostTwo(V2, L).
atMostTwo(_, []).

atMostTwo(V1, V2, [X|L]):-
	writeClause([ \+V1, \+V2, \+X ]),
	atMostTwo(L).
atMostTwo(_, _, []).

