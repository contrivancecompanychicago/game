SOURCES =  game.c cmd_params.c predator.c prey.c strategy_payoff.c produce_output.c
OBJECTS = $(SOURCES:%.c=%.o)

TARGET = game

all: $(TARGET)

%.o: %.c
	gcc -pedantic -g $< -c -o $@

$(TARGET): $(OBJECTS)
	gcc $(OBJECTS) -o $@ -lm -lgsl -lgslcblas

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f *.txt
