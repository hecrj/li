file = solver

$(file).out: $(file).pl ../writeClauses.pl displayFlow.pl test/*.pl
	swipl -O -g main --stand_alone=true -o $(file).out -c $(file).pl
