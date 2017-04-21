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
#include <condition_variable>
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
	bufferqueue(std::make_unique<std::array<T, length>>()), start_index(0), end_index(0), count(0),
	buffermutex(std::make_unique<std::mutex>()),hasSpace(std::make_unique<std::condition_variable>()),hasData(std::make_unique<std::condition_variable>())
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

		unique_lock<std::mutex> guard(*buffermutex);
		hasData->wait(guard, [this]{return count != 0;});
		if(count == 0) {
			throw std::exception();
		}

		std::cout << "EventList::start_index " << start_index << std::endl;
		std::cout << "EventList::end_index " << end_index << std::endl;
		std::cout << "EventList::count " << count << std::endl;
		T element = std::move(bufferqueue->at(start_index));
		start_index = (start_index + 1) % length;
		count--;
		hasSpace->notify_all();
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
	unique_ptr<std::mutex> buffermutex;
	unique_ptr<std::condition_variable> hasSpace;
	unique_ptr<std::condition_variable> hasData;
};

template<class T, size_t length>
void EventList<T, length>::push_back(T&& element) {
	std::cout << "EventList::push_back()" << std::endl;

	unique_lock<std::mutex> guard(*buffermutex);
	hasSpace->wait(guard, [this]{return count != length;});

		std::cout << "EventList::start_index " << start_index << std::endl;
		std::cout << "EventList::end_index " << end_index << std::endl;
		std::cout << "EventList::count " << count << std::endl;

	if (count == length) {
		throw std::exception();
	}
		bufferqueue->at(end_index);
		end_index = (end_index + 1) % length;
		count++;
	hasData->notify_all();
}


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
	Generator(EventList<T, length> * output):output(output){    // random-number engine used (Mersenne-Twister in this case)
	}

	void execute() {
		//Make random event
		std::cout << "Creating randomEventList" << std::endl;

		shared_ptr<EventList<T, length>> randomEventList = std::make_shared<EventList<T, length>>();
		int random_size = 1;

		std::generate_n(back_inserter(*randomEventList), random_size, makeRandomEvent);
	}
private:
	EventList<T, length> * output;

};

template<class T, size_t length>
class Display : public I_Filter{
public:
	Display(EventList<T, length> * input): input(input) {

	}
	void execute() {
		std::cout << "Beginning of displaying events events:" << std::endl;
		while (!input->empty()){
			std::cout << "events::count()" << input->getCount() << std::endl;
			T event = std::move(input->pop_front());
			std::cout << "Event start" << std::endl;
			std::cout << event.typeAsString() << std::endl;
			std::cout << event.what() << std::endl;
			std::cout << "Event end" << std::endl;
		}
		std::cout << "End of displaying events:" << std::endl;

	}
private:
	EventList<T, length> * input;
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
	//EventList<Event, 1> pipe;

	EventList<Event, 1> eventList;

	Generator<Event, 1> generator(&eventList);
	Display<Event, 1> display(&eventList);

	auto run_policy = [](I_Filter& runnable, std::chrono::milliseconds delay = 0ms) {
		while(true) runnable.execute();
		std::this_thread::sleep_for(delay);
	};
	std::thread generatorThread {run_policy, std::ref(generator), 2000ms};
	std::thread displaythread { run_policy, std::ref(display) };

	//generator.execute();
	//display.execute();

	while (true);
	cout << "!!!Goodbye World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
