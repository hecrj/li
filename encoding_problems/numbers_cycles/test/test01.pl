rows(9).
columns(9).

num(I, J, 1):-
	between(1, 9, I), between(1, 9, J).
