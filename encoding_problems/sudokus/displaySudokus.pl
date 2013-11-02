% Display sudokus
writeSeparator(0).
writeSeparator(N):-
	write('-'),
	N1 is N-1,
	writeSeparator(N1), !.

separateRow:- write(' '), writeSeparator(19), nl.
displaySol(S):- cells(Rows, Cols),
	between(1, Rows, I), between(1, Cols, J),
	between(1, 9, N),
	
	var2num(x-I-J-N, Vn),
	member(Vn, S),
	
	Im is mod(I-1, 3),
	(Im = 0, J = 1 -> separateRow ; true),
	
	Jm is mod(J-1, 3),
	(Jm = 0 -> write(' | ') ; true),
 
	write(N),
	
	(J = 9 -> write(' |'), nl ; true),
	fail.
displaySol(_):- separateRow.
