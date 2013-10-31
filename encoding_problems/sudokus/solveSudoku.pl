:-include('test/sud78').
:-include('../writeClauses').
:-dynamic(varNumber/3).
symbolicOutput(0). % set to 1 to see symbolic output only; 0 otherwise.
cells(9, 9).

writeClauses:- atleastOneNumberPerCell, atmostOneNumberPerCell,
	atmostOneOccurrencePerRow, atmostOneOccurrencePerCol,
	atmostOneOccurrencePerSquare, partiallyFilled.

atleastOneNumberPerCell:- cells(Rows, Cols), between(1, Rows, I), between(1, Cols, J),
	findall( x-I-J-N, between(1, 9, N), C ), writeClause(C), fail.
atleastOneNumberPerCell.

atmostOneNumberPerCell:- cells(Rows, Cols), between(1, Rows, I), between(1, Cols, J),
	between(1, 9, N1), between(1, 9, N2), N1 < N2,
	writeClause( [ \+x-I-J-N1, \+x-I-J-N2 ] ), fail.
atmostOneNumberPerCell.

atmostOneOccurrencePerRow:- cells(Rows, Cols), between(1, Rows, I), between(1, 9, N),
	between(1, Cols, J1), between(1, Cols, J2), J1 < J2,
	writeClause( [ \+x-I-J1-N, \+x-I-J2-N ] ), fail.
atmostOneOccurrencePerRow.

atmostOneOccurrencePerCol:- cells(Rows, Cols), between(1, Cols, J), between(1, 9, N),
	between(1, Rows, I1), between(1, Rows, I2), I1 < I2,
	writeClause( [ \+x-I1-J-N, \+x-I2-J-N ] ), fail.
atmostOneOccurrencePerCol.

atmostOneOccurrencePerSquare:- between(0, 2, SI), between(0, 2, SJ), between(1, 9, N),
	Is is SI * 3,
	Js is SJ * 3,

	between(0, 2, I1s), between(0, 2, J1s),
	I1 is Is + I1s + 1,
	J1 is Js + J1s + 1,

	between(0, 2, I2s), between(0, 2, J2s),
	I1s < I2s, J1s \= J2s,

	I2 is Is + I2s + 1,
	J2 is Js + J2s + 1,

	writeClause( [ \+x-I1-J1-N, \+x-I2-J2-N ] ), fail.
atmostOneOccurrencePerSquare.

partiallyFilled:- filled(I, J, N),
	writeClause( [ x-I-J-N ] ), fail.
partiallyFilled.

writeSeparator(0).
writeSeparator(N):-
	write('-'),
	N1 is N-1,
	writeSeparator(N1).

displaySol([]):- write(' '), writeSeparator(19), nl.
displaySol([Nv|S]):-
	num2var(Nv,x-I-J-N),
 
	Im is mod(I-1, 3),
	(Im \= 0; J \= 1; displaySol([])),
 
	Jm is mod(J-1, 3),
	(Jm \= 0; write(' | ')),
 
	write(N),
	
	(J \= 9; write(' |'), nl),
	displaySol(S).
