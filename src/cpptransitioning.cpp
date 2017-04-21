//============================================================================
// Name        : cpptransitioning.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <random>
using namespace std;
static const char * conditionAsString[] { "Warning", "Caution", "Advisory" };

class Event {
public:
	Event() {
		std::cout << "Empty constructor ran." << std::endl;
	}
	enum class Condition {
		WARNING, CAUTION, ADVISORY
	};
	Event(Condition condition) :
			condition(condition) {
		std::cout << "Event constructor ran." << std::endl;
	}
	~Event() {
		std::cout << "Event destructor ran." << std::endl;
	}

	Condition type() const {
		return condition;
	}

	const char * typeAsString() {
		return conditionAsString[static_cast<int>(condition)];
	}

private:
	Condition condition;
};

class Pipe {
public:
	void push(Event event) {
		storage = event;
	}
	Event pull() {
		return storage;
	}
private:
	Event storage;
};
static std::random_device rd;     // only used once to initialise (seed) engine
class Generator {
public:
	Generator(Pipe * output):output(output),
		rng(rd()){    // random-number engine used (Mersenne-Twister in this case)
	}
	void execute() {
		//Make random event
		std::uniform_int_distribution<int> uni(0,3); // guaranteed unbiased

		int random_integer = uni(rng);
		Event::Condition randomCondition(static_cast<Event::Condition>(random_integer));
		Event randomEvent(randomCondition);
		output->push(randomEvent);
	}
private:

	Pipe * output;
	std::mt19937 rng;    // random-number engine used (Mersenne-Twister in this case)
};

class Display {
public:
	Display(Pipe * input): input(input) {

	}
	void execute() {
		Event event = input->pull();
		std::cout << "Displaying event: " << event.typeAsString() << std::endl;
	}
private:
	Pipe * input;
};

int main() {

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	Pipe pipe;
	Generator generator(&pipe);
	Display display(&pipe);

	for (auto i : {0}) {
		generator.execute();
		display.execute();
	}

	return 0;
}
