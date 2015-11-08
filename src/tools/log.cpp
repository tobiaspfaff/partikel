// Basic logging

#include "tools/log.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

void fatalError(const string& msg) 
{
	cerr << msg << endl;
#ifdef NDEBUG
	exit(1);
#else
	raise(SIGSEGV);
#endif
}