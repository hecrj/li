file = puente

$(file).out: $(file).pl ../solver.pl
	swipl -O -g main --stand_alone=true -o $(file).out -c $(file).pl
