CC=gcc
CFLAGS= -g -Wall -Wextra -std=gnu11
OUTPUT=shell

help:
	@echo "Small bash"
	@echo ""
	@echo "Possiblities in the makefile :"
	@echo -e " - help\t\t: Shows this help page (default)"
	@echo -e " - shell\t: Compiles the interpretor"
	@echo -e " - run\t\t: Runs the interpretor"
	@echo -e " - clean\t: Cleans everything that was created"

run: $(OUTPUT)
	./$(OUTPUT)

$(OUTPUT): $(OUTPUT).c binary_tree.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) $< -c

clean:
	rm -f *.o $(OUTPUT) PremierRapport/Rapport1.{a,b,l,s}* SecondRapport/Rapport2.{a,b,l,s}*

.PHONY: clean run