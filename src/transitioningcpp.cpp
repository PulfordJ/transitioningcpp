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

template<class T, size_t length>
class EventList {
public:
	EventList():
	bufferqueue(std::make_unique<std::array<T, length>>()), start_index(0), end_index(0), count(0)
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
		T element = std::move(bufferqueue->at(start_index));
		start_index = start_index++ % length;
		count--;

		return element;
	}

	bool empty() {
		return count == 0;
	}

private:
	unique_ptr<std::array<T, length>> bufferqueue;

	size_t start_index;
	size_t end_index;
	size_t count;

};

template<class T, size_t length>
void EventList<T, length>::push_back(T&& element) {
	if (count == length) {
		throw std::exception();
	}
		bufferqueue->at(end_index) = std::move(element);
		end_index = end_index++ % length;
		count++;
}


template<class T, size_t length>
class Pipe {
public:
	void push(EventList<T, length>&& event) {
		storage = std::move(event);
	}
	EventList<T, length> pull() {
		return std::move(storage);
	}
private:
	EventList<T, length> storage;
};

static std::random_device rd;     // only used once to initialise (seed) engine

template<class T, size_t length>
class Generator : public I_Filter {
public:
	Generator(Pipe<T, length> * output):output(output),
		rng(rd()){    // random-number engine used (Mersenne-Twister in this case)
	}

	void execute() {
		//Make random event
		std::uniform_int_distribution<int> uni(1,2); // guaranteed unbiased
		std::cout << "Creating randomEventList" << std::endl;
		EventList<T, length> randomEventList;;
		int random_size = 1;//uni(rng);

		for (auto i = 0; i < random_size; i++) {
			int random_integer = uni(rng);
			Event::Condition randomCondition(static_cast<Event::Condition>(random_integer));
			std::string desc(std::string("Event no :") +std::to_string(i));
			std::unique_ptr<Event> randomEvent = std::make_unique<Event>(randomCondition, desc.c_str());
			//Event(randomCondition, desc.c_str());
			randomEventList.push_back(std::move(randomEvent));
		}

		output->push(std::move(randomEventList));
	}
private:
	Pipe<T, length> * output;
	std::mt19937 rng;    // random-number engine used (Mersenne-Twister in this case)
};

template<class T, size_t length>
class Display : public I_Filter{
public:
	Display(Pipe<T, length> * input): input(input) {

	}
	void execute() {
		std::cout << "Beginning of displaying events events:" << std::endl;
		EventList<T, length> events = input->pull();
		while (!events.empty()){
			T event = std::move(events.pop_front());
			std::cout << "Event start" << std::endl;
			std::cout << event->typeAsString() << std::endl;
			std::cout << event->what() << std::endl;
			std::cout << "Event end" << std::endl;
		}
		std::cout << "End of displaying events:" << std::endl;

	}
private:
	Pipe<T, length> * input;
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
	Pipe<unique_ptr<Event>, 2> pipe;
	Generator<unique_ptr<Event>, 2> generator(&pipe);
	Display<unique_ptr<Event>, 2> display(&pipe);

	Pipeline pipeline;
	pipeline.add(generator);
	pipeline.add(display);

	pipeline.run();
	cout << "!!!Goodbye World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
