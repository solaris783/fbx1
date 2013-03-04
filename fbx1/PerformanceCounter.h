#ifndef _PERFORMANCE_COUNTER_H


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>

using namespace std;

inline unsigned __int64 RDTSC(void)
{
	_asm  _emit 0x0F
	_asm  _emit 0x31
}

class TimerRDTSC
{
	private:
		unsigned __int64  start_cycle;
		unsigned __int64  end_cycle;

	public:
		TimerRDTSC() { start_cycle = 0; end_cycle = 0; }

		inline void Start()
		{
			start_cycle = RDTSC();
		}

		inline void Stop()
		{
			end_cycle = RDTSC();
		}

		unsigned __int64 Interval()
		{
			return end_cycle - start_cycle;
		}
};

class TimerPerformanceCounter
{
	private:
		unsigned __int64  start_time;
		unsigned __int64  end_time;

	public:
		TimerPerformanceCounter() {start_time=0; end_time=0;}

		inline void Start()
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start_time));
		}

		inline void Stop()
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&end_time));
		}

		unsigned __int64 Interval()
		{
			// Return duration in seconds...
			return end_time - start_time;
		}
};

template<class Timer, class Test, unsigned SleepRepeat, unsigned QuantumRepeat=1>
class ProfileSpeed
{
	unsigned __int64 test_interval;
	Timer            timer;
	Test             test;

	void QuantumTest()
	{
		unsigned i;
		Sleep(10);
		for (i=0; i < QuantumRepeat; ++i)
		{
			timer.Start();
			test.RunTest();
			timer.Stop();
			test_interval += timer.Interval();
		}
	}

	public:
		ProfileSpeed() : test_interval(0) {}

		void Run()
		{
			unsigned i;
			for (i=0; i < SleepRepeat; ++i)
			{
				QuantumTest();
			}
		}

		unsigned __int64 TestInterval() {return test_interval;}
};


#if 0
class HelloTest
{
	public:
	inline static void RunTest()
	{
		cout << "Hello world!" << endl;
	}
};


int main(int argc, char* argv[])
{
  // Run HelloTest 50 times sleeping between each test, using TimerRDTSC
  ProfileSpeed<TimerRDTSC, HelloTest, 50> test1;

  // Run HelloTest 50 times sleeping between every 2 tests, using TimerRDTSC
  ProfileSpeed<TimerRDTSC, HelloTest, 25, 2> test2;

  // Switch test order if an argument is specified to the program
  if (argc == 1)
  {
    test1.Run();
    test2.Run();
  }
  else
  {
    test2.Run();
    test1.Run();
  }

  cout << "Test 1 Interval: " << unsigned(test1.TestInterval()) << endl;
  cout << "Test 2 Interval: " << unsigned(test2.TestInterval()) << endl;

  return 0;
}
#endif   // if 0

#endif   // _PERFORMANCECOUNTER_H
