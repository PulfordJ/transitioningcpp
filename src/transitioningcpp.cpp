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
#include<utility>
#include <memory>
using namespace std;
static const char * conditionAsString[] { "Warning", "Caution", "Advisory" };

class I_Filter {
public:
	virtual void execute() = 0;
};

#include <string.h>
class Event {
public:
	Event(Event&& event) noexcept {
		this->condition = Event::Condition::WARNING;
		this->description = nullptr;
		std::cout << "Event::(Event&&)" << std::endl;
		swap(*this, event);
	}
	void swap(Event& lhs, Event& rhs) {
		using std::swap;
		swap(lhs.condition, rhs.condition);
		swap(lhs.description, rhs.description);
	}
	Event& operator=(Event other) {
		std::cout << "Event&::operator=(Event)" << std::endl;
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
		std::cout << "Event::Event()" << std::endl;

	}
	~Event() {
		std::cout << "Event::~Event()" << std::endl;
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

#include <queue>

template<class T>
class EventList {
public:
	EventList() {
		std::cout << "EventList::EventList()" << std::endl;
	}
	~EventList() {
			std::cout << "EventList::~EventList()" << std::endl;
	}

	void push_back(T element);

	T pop_front() {
		T element = std::move(bufferqueue.front());
		bufferqueue.pop();
		return element;
	}
	bool empty() {
		return bufferqueue.empty();
	}

private:
	std::queue<T> bufferqueue;

};

template<class T>
void EventList<T>::push_back(T element) {
	bufferqueue.emplace(std::move(element));
}


template<class T>
class Pipe {
public:
	void push(std::shared_ptr<EventList<T>> event) {
		storage = event;
	}
	std::shared_ptr<EventList<T>> pull() {
		return storage;
	}
private:
	std::shared_ptr<EventList<T>> storage;
};

static std::random_device rd;     // only used once to initialise (seed) engine

template<class T>
class Generator : public I_Filter {
public:
	Generator(Pipe<T> * output):output(output),
		rng(rd()){    // random-number engine used (Mersenne-Twister in this case)
	}

	void execute() {
		//Make random event
		std::uniform_int_distribution<int> uni(1,2); // guaranteed unbiased
		std::cout << "Creating randomEventList" << std::endl;
		std::shared_ptr<EventList<T>> randomEventList = std::make_shared<EventList<T>>();
		int random_size = 1;//uni(rng);

		for (auto i = 0; i < random_size; i++) {
			int random_integer = uni(rng);
			Event::Condition randomCondition(static_cast<Event::Condition>(random_integer));
			std::string desc(std::string("Event no :") +std::to_string(i));
			std::shared_ptr<Event> randomEvent = std::make_shared<Event>(randomCondition, desc.c_str());
			//Event(randomCondition, desc.c_str());
			randomEventList->push_back(randomEvent);
		}

		output->push(randomEventList);
	}
private:
	Pipe<T> * output;
	std::mt19937 rng;    // random-number engine used (Mersenne-Twister in this case)
};

template<class T>
class Display : public I_Filter{
public:
	Display(Pipe<T> * input): input(input) {

	}
	void execute() {
		std::cout << "Beginning of displaying events events:" << std::endl;
		std::shared_ptr<EventList<T>> events = input->pull();
		while (!events->empty()){
			T event = std::move(events->pop_front());
			std::cout << "Event start" << std::endl;
			std::cout << event->typeAsString() << std::endl;
			std::cout << event->what() << std::endl;
			std::cout << "Event end" << std::endl;
		}
		std::cout << "End of displaying events:" << std::endl;

	}
private:
	Pipe<T> * input;
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
	Pipe<shared_ptr<Event>> pipe;
	Generator<shared_ptr<Event>> generator(&pipe);
	Display<shared_ptr<Event>> display(&pipe);

	Pipeline pipeline;
	pipeline.add(generator);
	pipeline.add(display);

	pipeline.run();
	cout << "!!!Goodbye World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
