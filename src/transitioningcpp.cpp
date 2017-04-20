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
	Event(const Event& event):
		description(new char [strlen(event.description) + 1]){
		std::cout << "Copy constructor ran." << std::endl;
		this->condition = event.condition;
		//if (event.description != nullptr) {
			strncpy(this->description, event.description, strlen(event.description));
		//}
		this->description[strlen(event.description)] = '\0';
	}
	void swap(Event& lhs, Event& rhs) {
		using std::swap;
		swap(lhs.condition, rhs.condition);
		swap(lhs.description, rhs.description);
	}
	Event& operator=(Event other) {
		std::cout << "Assignment operator" << std::endl;
		swap(*this, other);
		return *this;
	}
	enum class Condition {
		WARNING, CAUTION, ADVISORY
	};
	Event(Condition condition, const char * description) :
			condition(condition),
			description(new char [strlen(description) + 1]) {
		strncpy(this->description, description, strlen(description));
		this->description[strlen(description)] = '\0';
		std::cout << "Event constructor ran." << std::endl;

	}
	~Event() {
		std::cout << "Event destructor ran." << std::endl;
		delete[] this->description;
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
		std::uniform_int_distribution<int> uni(1,2); // guaranteed unbiased


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
		std::cout << "Beginning of displaying events events:" << std::endl;
		EventList events = input->pull();
		for (Event& event : events) {
			std::cout << "Event start" << std::endl;
			std::cout << event.typeAsString() << std::endl;
			std::cout << event.what() << std::endl;
			std::cout << "Event end" << std::endl;
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
	cout << "!!!Goodbye World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
