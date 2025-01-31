#include "tests.h"
#include <iostream>
#include <unordered_map>
#include <random>
#include <vector>
#include <sstream>
#include <chrono>
#include <SupportLibrary/BasicTypes.h>
#include <conio.h>

#include <BaseLibrary/Serialization>
#include <BaseLibrary/Logging>

#include <mymath/mymath.h>

using namespace std;


void TestSerializer();
void TestLogger();
void TestLoggerConcurrency();


int PetiTestGeneric() {
	TestLoggerConcurrency();

	_getch();
	return 0;
}



void TestLogger() {
	exc::Logger logger;

	bool isOpen = logger.OpenFile("D:\\LOGFILE.log");
	if (!isOpen) {
		cout << "could not open logfile" << endl;
		return;
	}

	{
		exc::LogStream stream1, stream2;

		stream1 = logger.CreateLogStream("stream 1");
		stream2 = logger.CreateLogStream("stream 2");

		for (int i = 0; i < 20; i+=2) {
			stringstream ss;
			ss << "event " << i;
			stream1.Event(exc::Event{ ss.str() });
			ss.str(string());
			ss << "event " << i+1;
			stream2.Event(exc::Event{ ss.str() });
		}
	}

	cout << "done" << endl;
}

void TestLoggerConcurrency() {
	exc::LogCentre& logger = *exc::LogCentre::GetInstance();
	if (!logger.OpenFile("D:\\LOGFILE.log")) {
		cout << "could not open logfile" << endl;
		return;
	}

	std::thread threads[8];
	volatile unsigned threadId = 0;
	volatile int numPipesToCreate = 100000;
	volatile int numEventsToCreate = 100000;
	for (auto& t : threads) {
		t = std::thread([&] {
			unsigned myId = threadId++;

			// create a bunch of pipes concurrently
			int pipesLeft;
			while ((pipesLeft = --numPipesToCreate) >= 0) {
				exc::LogStream s = logger.CreateLogStream("pipe test");
				if (pipesLeft % 1000 == 0) {
					cout << "pipes left = " << pipesLeft << endl;
				}
			}

			// create a single pipe
			std::stringstream ss;
			ss << myId;
			exc::LogStream s = logger.CreateLogStream("stream " + ss.str());
			s.Event("hell� j�zsi", exc::eEventDisplayMode::STDOUT);

			// write a bunch of events to pipe
			int myEvent;
			while ((myEvent = --numEventsToCreate) >= 0) {
				ss.str(std::string());
				ss << myEvent;

				exc::Event evt("event " + ss.str(), 
							   exc::EventParameterInt("thread id", myId),													
							   exc::EventParameterFloat("pi", 3.1415926));
				s.Event(std::move(evt));

				if (myEvent % 1000 == 0) {
					cout << "events left = " << myEvent << endl;
				}
			}
		});
	}

	for (auto& t : threads) {
		t.join();
	}

	cout << "done" << endl;
}



void TestSerializer() {
	// serializer
	exc::BinarySerializer ser;

	char		i8_o = -127;
	short		i16_o = -32767;
	int			i32_o = -2147483647;
	long long	i64_o = -8796093022207;

	unsigned char		u8_o = 0xDEu;
	unsigned short		u16_o = 0xDEADu;
	unsigned int		u32_o = 0xDEADBEEFu;
	unsigned long long	u64_o = 0xDEADBEEF2B00B1E5;

	char		i8_r = 0;
	short		i16_r = 0;
	int			i32_r = 0;
	long long	i64_r = 0;

	unsigned char		u8_r = 0;
	unsigned short		u16_r = 0;
	unsigned int		u32_r = 0;
	unsigned long long	u64_r = 0;

	float f = 3.141592653f;
	float f_den = 1e-40f;
	float f_high = 1e+37f;
	float f_low = 1e-37f;
	float f_neg = -2.718281828f;

	double d = 3.1415926535897932384626;
	double d_den = 1e-310;
	double d_high = 1e+308;
	double d_low = 1e-308;
	double d_neg = -2.71828182845904;

	float f_r = 0;
	float f_den_r = 0;
	float f_high_r = 0;
	float f_low_r = 0;
	float f_neg_r = -0;

	double d_r = 0;
	double d_den_r = 0;
	double d_high_r = 0;
	double d_low_r = 0;
	double d_neg_r = -0;

	uint32_t uf = exc::FloatToIEEE754(f);
	float uf_r = exc::IEEE754ToFloat(uf);

	// write stuff to serializer
	ser << i8_o
		<< i16_o
		<< i32_o
		<< i64_o;

	ser << u8_o
		<< u16_o
		<< u32_o
		<< u64_o;

	ser << f
		<< f_den
		<< f_high
		<< f_low
		<< f_neg;

	ser << d
		<< d_den
		<< d_high
		<< d_low
		<< d_neg;

	// read stuff from serializer
	i8_r << ser;
	i16_r << ser;
	i32_r << ser;
	i64_r << ser;

	u8_r << ser;
	u16_r << ser;
	u32_r << ser;
	u64_r << ser;

	f_r << ser;
	f_den_r << ser;
	f_high_r << ser;
	f_low_r << ser;
	f_neg_r << ser;

	d_r << ser;
	d_den_r << ser;
	d_high_r << ser;
	d_low_r << ser;
	d_neg_r << ser;

	// check equality
	if (i8_r != i8_o) { cout << "error: " << 0 << endl; }
	if (i16_r != i16_o) { cout << "error: " << 1 << endl; }
	if (i32_r != i32_o) { cout << "error: " << 2 << endl; }
	if (i64_r != i64_o) { cout << "error: " << 3 << endl; }

	if (u8_r != u8_o) { cout << "error: " << 4 << endl; }
	if (u16_r != u16_o) { cout << "error: " << 5 << endl; }
	if (u32_r != u32_o) { cout << "error: " << 6 << endl; }
	if (u64_r != u64_o) { cout << "error: " << 7 << endl; }

	if (f_r != f) { cout << "error: " << 8 << endl; }
	if (f_den_r != f_den) { cout << "error: " << 9 << endl; }
	if (f_high_r != f_high) { cout << "error: " << 10 << endl; }
	if (f_low_r != f_low) { cout << "error: " << 11 << endl; }
	if (f_neg_r != f_neg) { cout << "error: " << 12 << endl; }

	if (d_r != d) { cout << "error: " << 13 << endl; }
	if (d_den_r != d_den) { cout << "error: " << 14 << endl; }
	if (d_high_r != d_high) { cout << "error: " << 15 << endl; }
	if (d_low_r != d_low) { cout << "error: " << 16 << endl; }
	if (d_neg_r != d_neg) { cout << "error: " << 17 << endl; }
}