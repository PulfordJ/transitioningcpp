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
#include <mutex>
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
	Event& operator=(Event&& other) {
		std::cout << "Event&::operator=(Event)" << std::endl;
		swap(*this, other);
		return *this;
	}
	enum class Condition {
		WARNING, CAUTION, ADVISORY
	};
	Event(): Event(Event::Condition::WARNING, "Warning.") {
		std::cout << "Event::Event()" << std::endl;
	}
	Event(Condition condition, const char * description) :
			condition(condition),
			description(new char [strlen(description) + 1]) {
		strncpy(this->description, description, strlen(description));
		this->description[strlen(description)] = '\0';
		std::cout << "Event::Event(Condition, const char *)" << std::endl;

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
	using value_type = typename std::array<T, length>::value_type;
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
		std::cout << "EventList::pop_front()" << std::endl;
		std::cout << "EventList::start_index " << start_index << std::endl;
		std::cout << "EventList::end_index " << end_index << std::endl;
		std::cout << "EventList::count " << count << std::endl;

		if(count == 0) {
			throw std::exception();
		}
		std::cout << "EvnetList::push_back() no exception" << std::endl;
		T element = std::move(bufferqueue->at(start_index));
		start_index = (start_index + 1) % length;
		count--;
		return element;
	}

	size_t getCount() {
		return count;
	}

	bool empty() {
		return count == 0;
	}

private:
	unique_ptr<std::array<Event, length>> bufferqueue;

	size_t start_index;
	size_t end_index;
	size_t count;
};

template<class T, size_t length>
void EventList<T, length>::push_back(T&& element) {
	std::cout << "EventList::push_back()" << std::endl;
			std::cout << "EventList::start_index " << start_index << std::endl;
			std::cout << "EventList::end_index " << end_index << std::endl;
			std::cout << "EventList::count " << count << std::endl;
	if (count == length) {
		throw std::exception();
	}
		bufferqueue->at(end_index);
		end_index = (end_index + 1) % length;
		count++;
}


template<class T, size_t length>
class Pipe {
public:
	void push(EventList<T, length>&& event) {
		std::lock_guard<std::mutex> lock(mutex);
		storage = std::move(event);
		empty = false;
	}
	EventList<T, length> pull() {
		std::lock_guard<std::mutex> lock(mutex);
		empty = true;
		return std::move(storage);

	}
	bool isEmpty() {
		std::lock_guard<std::mutex> lock(mutex);
		return empty;
	}
private:
	EventList<T, length> storage;
	bool empty = true;
	std::mutex mutex;
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
		int random_size = 1;

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
			std::cout << "events::count()" << events.getCount() << std::endl;
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
	Pipe<Event, 1> pipe;

	Generator<Event, 1> generator(&pipe);
	Display<Event, 1> display(&pipe);

	auto run_policy = [](I_Filter& runnable, std::chrono::milliseconds delay = 0ms) {
		while(true) runnable.execute();
		std::this_thread::sleep_for(delay);
	};
	std::thread generatorThread {run_policy, std::ref(generator), 2000ms};
	std::thread displaythread { run_policy, std::ref(display) };

	while (true);
	cout << "!!!Goodbye World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
