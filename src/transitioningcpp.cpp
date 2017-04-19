//============================================================================
// Name        : cpptransitioning.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <random>
#include <vector>
using namespace std;
static const char * conditionAsString[] { "Warning", "Caution", "Advisory" };

class Event {
public:
	Event() {
		std::cout << "Empty constructor ran." << std::endl;
	}
	Event(const Event& event) {
		std::cout << "Copy constructor ran." << std::endl;
		this->condition = event.condition;
	}
	Event& operator=(const Event& other) {
		std::cout << "Assignment constructor" << std::endl;
		this->condition = other.condition;
		return *this;
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

class EventList {
public:
	void push_back(Event event) {
		events.push_back(event);
	}
	std::vector<Event>::iterator begin() {
		return events.begin();
	}
	std::vector<Event>::iterator end() {
		return events.end();
	}

	using value_type = std::vector<Event>::value_type;

private:
	std::vector<Event> events;

};

class Pipe {
public:
	void push(EventList event) {
		storage = event;
	}
	EventList pull() {
		return storage;
	}
private:
	EventList storage;
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


		EventList randomEventList;
		int random_size = uni(rng);
		//randomEventList.reserve(random_size);

		for (auto i = 0; i < random_size; i++) {
			int random_integer = uni(rng);
			Event::Condition randomCondition(static_cast<Event::Condition>(random_integer));
			Event randomEvent(randomCondition);
			randomEventList.push_back(randomEvent);
		}
		output->push(randomEventList);
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
		EventList events = input->pull();
		std::cout << "Beginning of displaying events events:" << std::endl;
		for (auto& event : events) {
			std::cout << event.typeAsString() << std::endl;
		}
		std::cout << "End of displaying events:" << std::endl;

	}
private:
	Pipe * input;
};

int main() {

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	Pipe pipe;
	Generator generator(&pipe);
	Display display(&pipe);

	generator.execute();
	display.execute();


	return 0;
}
