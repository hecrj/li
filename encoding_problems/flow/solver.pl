:-include('test/entradaFlow14').
:-include('../writeClauses').
:-include(displayFlow).
:-dynamic(varNumber/3).
symbolicOutput(0).

writeClauses:- successors, parents, initials, distances, colors.

adjacentsToVertex(I, J, A):-
	I1 is I - 1,
	I2 is I + 1,
	J1 is J - 1,
	J2 is J + 1,
	size(N),
	(I > 1 -> append([], [s-I-J-I1-J], L1) ; L1 = []),
	(I < N -> append(L1, [s-I-J-I2-J], L2) ; L2 = L1),
	(J > 1 -> append(L2, [s-I-J-I-J1], L3) ; L3 = L2),
	(J < N -> append(L3, [s-I-J-I-J2], A)  ; A  = L3).
	
parentsToVertex(I, J, P):-
	adjacentsToVertex(I, J, A),
	findall(s-X-Y-I-J, member(s-I-J-X-Y, A), P).

successors:-
	size(N),
	between(1, N, I), between(1, N, J),
	adjacentsToVertex(I, J, A),
	(c(_, _,_, I, J) -> exactlyZero(A) ; exactlyOne(A, successor)),
	fail.
successors.

parents:-
	size(N),
	between(1, N, I), between(1, N, J),
	parentsToVertex(I, J, P),
	(c(_, I, J, _, _) -> exactlyZero(P) ; exactlyOne(P, parent)),
	fail.
parents.

colors:-
	oneColorPerCell,
	successorColor.
colors.

oneColorPerCell:-
	size(N),
	between(1, N, I), between(1, N, J),
	findall(col-I-J-C, c(C, _,_, _,_), L),
	atMostOne(L, color),
	fail.
oneColorPerCell.

successorColor:-
	size(N),
	c(C, _,_, _,_),
	between(1, N, I), between(1, N, J),
	(c(_, _,_, I, J) -> fail ; true),
	adjacentsToVertex(I, J, A),
	successorColor(C, A),
	fail.
successorColor.

successorColor(C, [s-I-J-A-B|L]):-
	writeClause([ \+col-I-J-C, \+s-I-J-A-B, col-A-B-C ]),
	writeClause([ \+col-A-B-C, \+s-I-J-A-B, col-I-J-C ]),
	successorColor(C, L).
successorColor(_, _, _, []).

initials:-
	c(C, I1, J1, I2, J2),
	writeClause([ col-I1-J1-C ]),
	writeClause([ d-I1-J1-0 ]),
	writeClause([ col-I2-J2-C ]),
	fail.
initials.

distances:-
	oneDistancePerCell,
	successorDistance,
	maxDistanceNoSuccessor.

oneDistancePerCell:-
	size(N),
	N2 is N*N,
	between(1, N, I), between(1, N, J),
	findall(d-I-J-D, between(0, N2, D), L),
	exactlyOne(L, distance),
	fail.
oneDistancePerCell.

successorDistance:-
	size(N),
	N2 is N*N - 1,
	between(1, N, I), between(1, N, J),
	adjacentsToVertex(I, J, A),
	between(0, N2, D),
	successorDistance(D, A),
	fail.
successorDistance.

successorDistance(D, [s-I-J-A-B|L]):-
	D1 is D + 1,
	writeClause([ \+d-I-J-D, \+s-I-J-A-B, d-A-B-D1 ]),
	successorDistance(D, L).
successorDistance(_, []).

maxDistanceNoSuccessor:-
	size(N),
	N2 is N*N,
	between(1, N, I), between(1, N, J),
	adjacentsToVertex(I, J, A),
	maxDistanceNoSuccessor(N2, A),
	fail.
maxDistanceNoSuccessor.

maxDistanceNoSuccessor(D, [s-I-J-A-B|L]):-
	writeClause([ \+d-I-J-D, \+s-I-J-A-B ]),
	maxDistanceNoSuccessor(D, L).
maxDistanceNoSuccessor(_, []).

exactlyZero([X|L]):-
	% All of them need to be false.
	writeClause([ \+X ]),
	exactlyZero(L).
exactlyZero([]).

exactlyOne(A, Aux):-
	writeClause(A),
	atMostOne(A, Aux).

exactlyTwo(A):-
	atMostTwo(A),
	atLeastTwo(A).
	
exactlyThree(A):-
	atMostThree(A),
	atLeastThree(A).

% At Most One
atMostOne([V1|L], Aux):-
	atMostOne(V1, L, Aux).
atMostOne([], _).

atMostOne(V1, [V2|L], Aux):-
	% Ladder encoding: O(L)
	writeClause([ \+V1, V1-Aux ]),
	writeClause([ \+V1-Aux, \+V2]),
	writeClause([ \+V1-Aux, V2-Aux]),
	atMostOne(V2, L, Aux).

atMostOne(V, [], Aux):-
	writeClause([ \+V, V-Aux ]).

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

