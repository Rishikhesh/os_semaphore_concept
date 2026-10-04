CFLAGS = -Wall -Wextra -O2 -pthread
BINS = producer_consumer teaching_assistant dining_philosopher

all: $(BINS)

producer_consumer: producer_consumer.c sem.h
	$(CC) $(CFLAGS) -o $@ $<
teaching_assistant: teaching_assistant.c sem.h
	$(CC) $(CFLAGS) -o $@ $<
dining_philosopher: Dining_Philosopher.c sem.h
	$(CC) $(CFLAGS) -o $@ $<

# each program asserts its invariants and exits non-zero if one breaks
test: all
	./producer_consumer > /dev/null && echo "producer_consumer OK"
	./teaching_assistant 6 > /dev/null && echo "teaching_assistant OK"
	./dining_philosopher 5 3 > /dev/null && echo "dining_philosopher OK"

clean:
	rm -f $(BINS)

.PHONY: all test clean
