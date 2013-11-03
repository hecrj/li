:-include('test/sud77').
:-dynamic(varNumber/3).
symbolicOutput(0). % set to 1 to see symbolic output only; 0 otherwise.
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

% Cardinality constraints
:-dynamic(auxId/1).

exactlyOne(A):-
	atLeastOne(A),
	atMostOne(A).
	
% At Least One
atLeastOne(A):-
	writeClause(A).

% At Most One
auxId(0). % Next auxiliar variables identifier
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

% Write clauses
main:- symbolicOutput(1), !, writeClauses, halt. % escribir bonito, no ejecutar
main:-  assert(numClauses(0)), assert(numVars(0)),
	tell(clauses), writeClauses, told,
	tell(header),  writeHeader,  told,
	unix('cat header clauses > infile.cnf'),
	unix('picosat -v -o model infile.cnf'),
	unix('cat model'),
	see(model), readModel(M), seen,
	displaySol(M),
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
