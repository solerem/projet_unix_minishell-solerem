CFLAGS=-Wall -g
LDFLAGS=
TESTDIR=tests


all: .init .hooksset testlex minishell 

analex.o: analex.c analex.h
	gcc ${CFLAGS} -c $<

minishell.o: minishell.c analex.h
	gcc ${CFLAGS} -c $<

testlex.o: testlex.c analex.h 
	gcc ${CFLAGS} -c $<

testlex: testlex.o analex.o .init .hooksset
	gcc ${LDFLAGS} -o $@ testlex.o analex.o

minishell: minishell.o analex.o .init .hooksset
	gcc ${LDFLAGS} -o $@ minishell.o analex.o

.hooksset:
	git config core.hooksPath .githooks
	touch .hooksset

.init: 
	@echo "Intializing UNIX projet base"
	./test_archive.sh init

test: minishell ${TESTDIR}/*.ms
	./runtest.sh ${TESTDIR}/*.ms

testarchive:
	./test_archive.sh

clean:
	rm -rf *.o *~ .minishell.tmp testlex minishell
