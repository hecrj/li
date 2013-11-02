PROBLEMS = sudokus flow numbers_cycles
FILES = $(addprefix standalone/, $(addsuffix .pl, $(PROBLEMS)))
NAME = $(basename $(@F))
SOLVER = $(NAME)/$(NAME).pl
EXE = standalone/$(NAME).out
TEST = $(shell cat $(SOLVER) | grep test | cut -d "'" -s -f 2).pl
DISPLAY = $(shell cat $(SOLVER) | grep display | cut -d "'" -s -f 2).pl

all: $(FILES)

clean:
	rm -f standalone/*.pl
	rm -f standalone/test/*.pl
	rm -f standalone/*.out

.SECONDEXPANSION:
$(FILES): $$(SOLVER) writeClauses-standalone.pl cardinalityConstraints.pl
	cat $(SOLVER) cardinalityConstraints.pl writeClauses-standalone.pl > $@
	sed -i "/\.\.\/writeClauses/d" $@
	sed -i "/\.\.\/cardinalityConstraints/d" $@
	cp $(NAME)/$(TEST) standalone/$(TEST)
	cp $(NAME)/$(DISPLAY) standalone/$(DISPLAY)
	swipl -O -g main --stand_alone=true -o $(EXE) -c $@
