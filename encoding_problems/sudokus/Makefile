file = solveSudoku

$(file).out: $(file).pl
	swipl -O -g main --stand_alone=true -o $(file).out -c $(file).pl
