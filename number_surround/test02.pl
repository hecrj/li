rows(9).
columns(9).

num(I, J, 1):-
	between(1, 9, I), between(1, 3, J).

num(I, J, 2):-
	between(1, 5, I), between(4, 9, J).
