:-include(test01).
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
	adjacents(I, J, A)
	atLeastTwo(A),
	atMostTwo(A).

atMostOne([X|L]):-
	writeNegated(X, L),
	atMostOne(L).
atMostOne([]).

atLeastTwo([X|L]):-
.

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
atMostTwo(V1, V2, [_]).

atMostTwo():-

writeNegated(V, [X|L]):-
	writeClause([ \+V, \+X]),
	writeNegated(X, L).
writeNegated(_, []).
