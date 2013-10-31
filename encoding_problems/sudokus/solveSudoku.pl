:-include(sud78).
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

% ========== No need to change the following: =====================================

main:- symbolicOutput(1), !, writeClauses, halt. % escribir bonito, no ejecutar
main:-  assert(numClauses(0)), assert(numVars(0)),
	tell(clauses), writeClauses, told,
	tell(header),  writeHeader,  told,
	unix('cat header clauses > infile.cnf'),
        unix('make mySat.out'),
	unix('./mySat.out < infile.cnf > model'),
	unix('cat model'),
	see(model), readModel(M), seen, displaySol(M),
	halt.

var2num(T,N):- hash_term(T,Key), varNumber(Key,T,N),!.
var2num(T,N):- retract(numVars(N0)), N is N0+1, assert(numVars(N)), hash_term(T,Key),
	assert(varNumber(Key,T,N)), assert( num2var(N,T) ), !.

writeHeader:- numVars(N),numClauses(C),write('p cnf '),write(N), write(' '),write(C),nl.

countClause:-  retract(numClauses(N)), N1 is N+1, assert(numClauses(N1)),!.
writeClause([]):- symbolicOutput(1),!, nl.
writeClause([]):- countClause, write(0), nl.
writeClause([Lit|C]):- w(Lit), writeClause(C),!.
w( Lit ):- symbolicOutput(1), write(Lit), write(' '),!.
w(\+Var):- var2num(Var,N), write(-), write(N), write(' '),!.
w(  Var):- var2num(Var,N),           write(N), write(' '),!.
unix(Comando):-shell(Comando),!.
unix(_).

readModel(L):- get_code(Char), readWord(Char,W), readModel(L1), addIfPositiveInt(W,L1,L),!.
readModel([]).

addIfPositiveInt(W,L,[N|L]):- W = [C|_], between(48,57,C), number_codes(N,W), N>0, !.
addIfPositiveInt(_,L,L).

readWord(99,W):- repeat, get_code(Ch), member(Ch,[-1,10]), !, get_code(Ch1), readWord(Ch1,W),!.
readWord(-1,_):-!, fail. %end of file
readWord(C,[]):- member(C,[10,32]), !. % newline or white space marks end of word
readWord(Char,[Char|W]):- get_code(Char1), readWord(Char1,W), !.
%========================================================================================