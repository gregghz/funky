COMPILER=`which clang || which gcc || which cc`
TARGETFUNKMACHINE=funk_machine
run: initial-tests tinkerage.funk
	./$(TARGETFUNKMACHINE) ./indispensibles/core.srfm -b ./tinkerage.funk

tinkerage.funk:
	echo '(print "Hello, world")' > tinkerage.funk

initial-tests: funky_tools
	./runtests.sh
	echo 'Checking for memory leaks...'
	cat test/*.in | valgrind ./$(TARGETFUNKMACHINE) ./indispensibles/core.srfm  2>&1 | grep 'no leaks are possible'

funky_tools: $(TARGETFUNKMACHINE)

$(TARGETFUNKMACHINE): *.h *.c Makefile
	$(COMPILER) -g -o $(TARGETFUNKMACHINE) texty.c griddy.c funky_data.c funky_env.c funky_eval.c funky_op.c funky_read.c funky_buffer.c funky_debug.c funky.c

stage: run
	./anticommentism.sh
	git status -s | grep -e '^[M ]M' | awk '{print $$2}' | xargs git stage

tinker: funky_tools
	./$(TARGETFUNKMACHINE) ./indispensibles/core.srfm -b tinkerage.funk

funky: funky_tools
	printf '#!/bin/sh\n./funk_machine ./indispensibles/core.srfm $$@' > funky
	chmod +x funky
