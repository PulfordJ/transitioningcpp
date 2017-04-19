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

class I_Filter {
public:
	virtual void execute() = 0;
};

#include <string.h>
class Event {
public:
	Event() {

	}
	Event(const Event& event, const char * description):
		description(new char [strlen(description)]){
		strncpy(this->description, description, strlen(description));
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
		//delete this->description;
	}

	Condition type() const {
		return condition;
	}
	const char * what() {
		return description;
	}

	const char * typeAsString() {
		return conditionAsString[static_cast<int>(condition)];
	}

private:
	Condition condition;
	char * description;
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
class Generator : public I_Filter {
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
			std::string desc(std::string("Event no :") +std::to_string(i));
			Event randomEvent(randomCondition, desc.c_str());

			randomEventList.push_back(randomEvent);
		}
		output->push(randomEventList);
	}
private:
	Pipe * output;
	std::mt19937 rng;    // random-number engine used (Mersenne-Twister in this case)
};

class Display : public I_Filter{
public:
	Display(Pipe * input): input(input) {

	}
	void execute() {
		EventList events = input->pull();
		std::cout << "Beginning of displaying events events:" << std::endl;
		for (auto& event : events) {
			std::cout << event.typeAsString() << std::endl;
			std::cout << event.what() << std::endl;

		}
		std::cout << "End of displaying events:" << std::endl;

	}
private:
	Pipe * input;
};

class Pipeline {
public:
	void run() {
		for (auto filter : filters){
			filter->execute();
		}
	}
	void add(I_Filter& filter) {
		filters.push_back(&filter);
	}
private:
	std::vector<I_Filter *> filters;
};

int main() {

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	Pipe pipe;
	Generator generator(&pipe);
	Display display(&pipe);

	Pipeline pipeline;
	pipeline.add(generator);
	pipeline.add(display);

	pipeline.run();

	return 0;
}
