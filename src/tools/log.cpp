// Basic logging

#include "tools/log.hpp"
#include <iostream>
using namespace std;

void fatalError(const string& msg) 
{
	cerr << msg << endl;
	exit(1);
}