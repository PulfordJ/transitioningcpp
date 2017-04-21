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
#include <algorithm>
#include <thread>
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
	Event() {
		std::cout << "Event::Event()" << std::endl;
	}
	Event(Condition condition, const char * description) :
			condition(condition),
			description(new char [strlen(description) + 1]) {
		strncpy(this->description, description, strlen(description));
		this->description[strlen(description)] = '\0';
		std::cout << "Event::Event(Condition, const char *)" << std::endl;

	}
	Event(const Event& other) :
		condition(other.condition),
		description(new char [strlen(other.description) + 1]) {
	strncpy(this->description, other.description, strlen(other.description));
	this->description[strlen(other.description)] = '\0';
	std::cout << "Event::Event(Event&)" << std::endl;

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

template<class T, size_t length>
class EventList {
public:
	using value_type = typename std::vector<T>::value_type;
	EventList():
	bufferqueue(std::make_unique<std::vector<T>>()), start_index(0), end_index(0), count(0)
	{
		std::cout << "EventList::EventList()" << std::endl;
	}
	~EventList() {
			std::cout << "EventList::~EventList()" << std::endl;
	}
	EventList(EventList&& other) = default;
	EventList& operator=(EventList&& other) = default;

	void push_back(T&& element);

	T pop_front() {
		if(count == 0) {
			throw std::exception();
		}
		T element = bufferqueue->at(0);
		bufferqueue->erase(bufferqueue->begin());
		count--;
		return element;
	}

	size_t getCount() {
		return count;
	}

	bool empty() {
		return count == 0;
	}

	auto begin() {
		return bufferqueue->begin();
	}

	std::vector<Event>::iterator end() {
		return bufferqueue->end();
	}

	void erase(std::vector<Event>::iterator a, std::vector<Event>::iterator b) {
		bufferqueue->erase(a, b);
		count = bufferqueue->size();
	}

private:
	unique_ptr<std::vector<Event>> bufferqueue;

	size_t start_index;
	size_t end_index;
	size_t count;

};

template<class T, size_t length>
void EventList<T, length>::push_back(T&& element) {
	if (count == length) {
		throw std::exception();
	}
		bufferqueue->push_back(element);
		count++;
}


template<class T, size_t length>
class Pipe {
public:
	void push(EventList<T, length>&& event) {
		storage = std::move(event);
		empty = false;
	}
	EventList<T, length> pull() {
		empty = true;
		return std::move(storage);

	}
	bool isEmpty() {
		return empty;
	}
private:
	EventList<T, length> storage;
	bool empty = true;
};

static std::random_device rd;     // only used once to initialise (seed) engine
static std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)



Event makeRandomEvent() {
	static int eventNum = 0;
	std::uniform_int_distribution<int> uni(0,2); // guaranteed unbiased
	int random_integer = uni(rng);
	Event::Condition randomCondition(static_cast<Event::Condition>(random_integer));
	std::string desc(std::string("Event no :") +std::to_string(++eventNum));
	Event randomEvent(randomCondition, desc.c_str());
	return randomEvent;
}


template<class T, size_t length>
class Generator : public I_Filter {
public:
	Generator(Pipe<T, length> * output):output(output){    // random-number engine used (Mersenne-Twister in this case)
	}

	void execute() {
		//Make random event
		std::cout << "Creating randomEventList" << std::endl;
		EventList<T, length> randomEventList;;
		int random_size = 10;

		std::generate_n(back_inserter(randomEventList), random_size, makeRandomEvent);

		output->push(std::move(randomEventList));
	}
private:
	Pipe<T, length> * output;

};

template<class T, size_t length>
class Display : public I_Filter{
public:
	Display(Pipe<T, length> * input): input(input) {

	}
	void execute() {
		std::cout << "Beginning of displaying events events:" << std::endl;
		if (input->isEmpty()) return;
		EventList<T, length> events = input->pull();
		while (!events.empty()){
			T event = std::move(events.pop_front());
			std::cout << "Event start" << std::endl;
			std::cout << event.typeAsString() << std::endl;
			std::cout << event.what() << std::endl;
			std::cout << "Event end" << std::endl;
		}


		std::cout << "End of displaying events:" << std::endl;

	}
private:
	Pipe<T, length> * input;
};

template<class T, size_t length>
class IDFilter: public I_Filter {
public:
	IDFilter(Pipe<T, length> * input, Pipe<T, length> * output): input(input), output(output) {

	}
	void execute() {
		if (!input->isEmpty()) {
			EventList<T, length> events = input->pull();
			vector<Event>::iterator it = std::remove_if(events.begin(), events.end(), [](Event event){return event.type() != Event::Condition::WARNING;});
			events.erase(it, events.end());
			output->push(std::move(events));
		}
	}
private:
	Pipe<T, length> * input;
	Pipe<T, length> * output;
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

#include <chrono>
int main() {

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	Pipe<Event, 10> pipe;
	Generator<Event, 10> generator(&pipe);

	Pipe<Event, 10> pipeIdFilterToDisplay;
	IDFilter<Event, 10> idFilter(&pipe, &pipeIdFilterToDisplay);
	Display<Event, 10> display(&pipeIdFilterToDisplay);

	//Pipeline pipeline;
	//generator.execute();


	auto run_policy = [](I_Filter& runnable, std::chrono::milliseconds delay = 0ms) {
		while(true) runnable.execute();
		std::this_thread::sleep_for(delay);
	};
	std::thread generatorThread {run_policy, std::ref(generator), 2000ms};
	std::thread displaythread { run_policy, std::ref(display) };
	std::thread idFilterThread { run_policy, std::ref(idFilter) };

	//display.execute();

	//pipeline.run();

	while (true);
	cout << "!!!Goodbye World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
