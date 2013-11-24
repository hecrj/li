:-include('test/entradaFlow14').
:-include('../writeClauses').
:-include('../cardinalityConstraints').
:-include('displayFlow').
symbolicOutput(0).
useMySolver(0). % Set this to 1 to use my sat solver

writeClauses:- successors, parents, initials, colors, distances.

up(1, _, []):- !.
up(I, J, [s-I-J-I1-J]):- I1 is I-1.

left(_, 1, []):- !.
left(I, J, [s-I-J-I-J1]):- J1 is J-1.

down(I, _, []):- size(N), I >= N, !.
down(I, J, [s-I-J-I2-J]):- I2 is I+1.

right(_, J, []):- size(N), J >= N, !.
right(I, J, [s-I-J-I-J2]):- J2 is J+1.

% Relation between a vertex (I, J) and its possible successors
adjacentsToVertex(I, J, A):-
	up(I, J, U),
	down(I, J, D),
	left(I, J, L),
	right(I, J, R),
	
	append(U, D, L1),
	append(L1, L, L2),
	append(L2, R, A).

% Relation between a vertex (I, J) and its possible parents
parentsToVertex(I, J, P):-
	adjacentsToVertex(I, J, A),
	findall(s-X-Y-I-J, member(s-I-J-X-Y, A), P).

% Successor constraint
% Given a vertex v:
% - v is an endpoint -> v must have exactly 0 successors
% - v is not an endpoint -> v must have exactly 1 successor
successors:-
	size(N),
	between(1, N, I), between(1, N, J),
	adjacentsToVertex(I, J, A),
	successors(I, J, A),
	fail.
successors.

successors(I, J, A):- c(_, _,_, I, J), exactlyZero(A), !.
successors(_, _, A):- exactlyOne(A).

% Parent constraint
% Given a vertex v:
% - v is an startpoint -> v must have exactly 0 parents
% - v is not an startpoint -> v must have exactly 1 parent
parents:-
	size(N),
	between(1, N, I), between(1, N, J),
	parentsToVertex(I, J, P),
	parents(I, J, P),
	fail.
parents.

parents(I, J, P):- c(_, I, J, _, _), exactlyZero(P), !.
parents(_, _, P):- exactlyOne(P).

% Color constraint
% 1. There must be exactly one color per vertex
% 2. Given u, v vertices:
%    successor(u) = v -> color(u) = color(v) 
colors:-
	oneColorPerVertex,
	successorColor.
colors.

oneColorPerVertex:-
	size(N),
	between(1, N, I), between(1, N, J),
	findall(col-I-J-C, c(C, _,_, _,_), L),
	exactlyOne(L),
	fail.
oneColorPerVertex.

successorColor:-
	size(N),
	c(C, _,_, _,_),
	between(1, N, I), between(1, N, J),
	not(c(_, _,_, I, J)),
	adjacentsToVertex(I, J, A),
	successorColor(C, A),
	fail.
successorColor.

% Given a color C and a vertex v
% (color(v) = C and successor(v) = u) -> color(u) = C 
successorColor(C, [s-I-J-A-B|L]):-
	writeClause([ \+col-I-J-C, \+s-I-J-A-B, col-A-B-C ]),
	writeClause([ \+col-A-B-C, \+s-I-J-A-B, col-I-J-C ]),
	successorColor(C, L).
successorColor(_, _, _, []).

% Sets the initial vertices colors and distances
initials:-
	c(C, I1, J1, I2, J2),
	writeClause([ col-I1-J1-C ]),
	writeClause([ d-I1-J1-0 ]),
	writeClause([ col-I2-J2-C ]),
	fail.
initials.

% Distance constraints (avoids loops)
% 1. Every vertex must have exactly one distance
% 2. Given vertices u, v:
%    (distance(u) = D and successor(u) = v) -> distance(v) = D+1
% 3. Given vertex u, and N = size
%    distance(u) = N*N -> u can not have a successor
distances:-
	oneDistancePerVertex,
	successorDistance,
	maxDistanceNoSuccessor.

oneDistancePerVertex:-
	size(N),
	N2 is N*N,
	between(1, N, I), between(1, N, J),
	findall(d-I-J-D, between(0, N2, D), L),
	exactlyOne(L),
	fail.
oneDistancePerVertex.

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
