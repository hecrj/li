file = solver

$(file).out: $(file).pl writeClauses.pl displayRodear.pl test*.pl
	swipl -O -g main --stand_alone=true -o $(file).out -c $(file).pl

mySat.out: ../main.cpp
	g++ ../main.cpp -O3 -DVERBOSE -o mySat.out
