// MT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "thread.h"
#include <string>
#include <list>
#include <iostream>
#include <sstream>

using namespace std;

/// <summary>
/// What is wrong in this multithreaded application.  
/// </summary>
class msg_archive : public mtlib::thread_t {
public:
	msg_archive() {
	}

	// a method is called from thread's callback within time interval in thread_t::start(timeout). 
	virtual bool on_execute() {
		stringstream msg;
		msg << "message" << m_count++;
		m_messages.push_back(msg.str());
		return true; 
	}

	bool get_messages(list<string>& messages) {
		if (m_messages.empty())
			return false;
		messages.assign(m_messages.begin(), m_messages.end());
		m_messages.clear();
		return true;
	}

private:
	list<string> m_messages;
	int m_count = 0;
};

class msg_display {
public:
	msg_display() {
	}
	void show_messages(const list<string>& messages) {
		for (auto msg : messages) {
			cout << msg << endl;
		}
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	msg_archive archive;
	archive.start(100);

	msg_display display;

	while (true) {
		list<string> messages;
		if (archive.get_messages(messages))
			display.show_messages(messages);
		Sleep(1000);
	}

	return 0;
}

