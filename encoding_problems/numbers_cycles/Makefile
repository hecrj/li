file = numbers_cycles

$(file).out: $(file).pl ../writeClauses.pl displayRodear.pl test/*.pl
	swipl -O -g main --stand_alone=true -o $(file).out -c $(file).pl
