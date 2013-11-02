:-include('test/sud78').
:-include('displaySudokus').
:-include('../writeClauses').
:-include('../cardinalityConstraints').
:-dynamic(varNumber/3).
symbolicOutput(0). % set to 1 to see symbolic output only; 0 otherwise.
cells(9, 9).

writeClauses:-
	oneNumberPerCell,
	oneOccurrencePerRow,
	oneOccurrencePerCol,
	atMostOneOccurrencePerSquare,
	partiallyFilled.

oneNumberPerCell:- cells(Rows, Cols),
	between(1, Rows, I), between(1, Cols, J),
	findall( x-I-J-N, between(1, 9, N), C ),
	exactlyOne(C),
	fail.
oneNumberPerCell.

oneOccurrencePerRow:- cells(Rows, Cols),
	between(1, Rows, I), between(1, 9, N),
	findall(x-I-J-N, between(1, Cols, J), L),
	exactlyOne(L),
	fail.
oneOccurrencePerRow.

oneOccurrencePerCol:- cells(Rows, Cols),
	between(1, Cols, J), between(1, 9, N),
	findall(x-I-J-N, between(1, Rows, I), L),
	exactlyOne(L),
	fail.
oneOccurrencePerCol.

atMostOneOccurrencePerSquare:-
	between(0, 2, SI), between(0, 2, SJ), between(1, 9, N),
	Istart is SI * 3 + 1,
	Jstart is SJ * 3 + 1,
	Iend is Istart + 2,
	Jend is Jstart + 2,
	
	between(Istart, Iend, I1), between(Jstart, Jend, J1),
	findall(x-I2-J2-N,
		(between(Istart, Iend, I2),
		between(Jstart, Jend, J2),
		I1 < I2, J1 \= J2),
		L),
		
	append([x-I1-J1-N], L, C),
	atMostOne(C),
	fail.
atMostOneOccurrencePerSquare.

partiallyFilled:- filled(I, J, N),
	writeClause( [ x-I-J-N ] ),
	fail.
partiallyFilled.

