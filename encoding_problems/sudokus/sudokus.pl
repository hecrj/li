:-include('test/sud77').
:-include('../writeClauses').
:-include('../cardinalityConstraints').
:-include('displaySudokus').
symbolicOutput(0). % set to 1 to see symbolic output only; 0 otherwise.
useMySolver(0). % Set this to 1 to use my sat solver

cells(9, 9).

writeClauses:-
	oneNumberPerCell,
	oneOccurrencePerRow,
	oneOccurrencePerCol,
	atMostOneOccurrencePerSquare,
	fill.

oneNumberPerCell:- cells(Rows, Cols),
	between(1, Rows, I), between(1, Cols, J),
	findall(x-I-J-N, between(1, 9, N), C),
	exactlyOne(C),
	fail.
oneNumberPerCell.

oneOccurrencePerRow:- cells(Rows, Cols),
	between(1, Rows, I), between(1, 9, N),
	findall(x-I-J-N, between(1, Cols, J), L),
	atLeastOne(L),
	atMostOne(L), % Redundant, but helps the SAT solver
	fail.
oneOccurrencePerRow.

oneOccurrencePerCol:- cells(Rows, Cols),
	between(1, Cols, J), between(1, 9, N),
	findall(x-I-J-N, between(1, Rows, I), L),
	atLeastOne(L),
	atMostOne(L), % Redundant, but helps the SAT solver
	fail.
oneOccurrencePerCol.

% I found that adding redundancy, in this case, helps to find
% a solution faster. SAT solvers can use the redundant constraints
% to perform better decisions:
%
% Test: sud77
% With redundancy:
% c 40 conflicts
% c 6 decisions
% Without redundancy:
% c 1729 conflicts
% c 3566 decisions
%
% And with an empty board:
% With redundancy:
% c 2 conflicts
% c 112 decisions
% Without redundancy:
% c 13754 conflicts
% c 29071 decisions
%
% This is, actually, pretty interesting because it proves that the
% smallest reductions may not be the best ones.

atMostOneOccurrencePerSquare:-
	between(0, 2, SI), between(0, 2, SJ),
	Istart is SI * 3 + 1,
	Jstart is SJ * 3 + 1,
	Iend is Istart + 2,
	Jend is Jstart + 2,
	
	between(1, 9, N),
	between(Istart, Iend, I1), between(Jstart, Jend, J1),
	between(Istart, Iend, I2), between(Jstart, Jend, J2),
	I1 < I2, J1 \= J2,
	
	% x-I1-J1-N -> -x-I2-J2-N
	writeClause([ \+x-I1-J1-N, \+x-I2-J2-N ]),
	fail.
atMostOneOccurrencePerSquare.

fill:- filled(I, J, N),
	writeClause( [ x-I-J-N ] ),
	fail.
fill.
