# Compilation Options:														#
# 	  Command					Action										#
# ------------------------------------------------------------------------- #
# >> make (all) 	=> both parts and dependencies							#
# >> make A			=> lsh, hypercube and their dependencies				#
# >> make lsh		=> lsh and deps											#
# >> make cube		=> hypercube and deps									#
# >> make cluster	=> kmeans, clustering and dependencies					#
# >> make clean		=> removes all object and all executable files			#
#############################################################################

SDIR = ./src
IDIR = ./inc
ODIR = ./obj
BDIR = ./bin
CC = g++
CFLAGS = -g -Wall -I$(IDIR)


_DEPS = error.hpp item.hpp hashtable.hpp tree.hpp utilities.hpp lsh.hpp hypercube.hpp kmeans.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))


$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	@mkdir -p ./obj/
	@$(CC) -c -o $@ $< $(CFLAGS)

all: A cluster

A: lsh cube

lsh: $(ODIR)/main_lsh.o $(ODIR)/utilities.o  $(ODIR)/kmeans.o $(ODIR)/hypercube.o $(ODIR)/lsh.o $(ODIR)/hashtable.o $(ODIR)/tree.o $(ODIR)/item.o
	@echo creating lsh executable..
	@mkdir -p ./bin/
	@$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)

cube: $(ODIR)/main_cube.o $(ODIR)/utilities.o  $(ODIR)/kmeans.o $(ODIR)/hypercube.o $(ODIR)/lsh.o $(ODIR)/hashtable.o $(ODIR)/tree.o $(ODIR)/item.o
	@echo creating hypercube executable..
	@mkdir -p ./bin/
	@$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)

cluster: $(ODIR)/main_cluster.o $(ODIR)/kmeans.o $(ODIR)/utilities.o $(ODIR)/hypercube.o $(ODIR)/lsh.o $(ODIR)/hashtable.o $(ODIR)/tree.o $(ODIR)/item.o
	@echo creating clustering executable..
	@mkdir -p ./bin/
	@$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)


.PHONY: clean

clean:
	@echo Removing object files and executable files
	@rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(BDIR)/*
	@echo Done!
