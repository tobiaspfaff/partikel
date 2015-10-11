#include <iostream>
#include <memory>
#include "render/init.hpp"

using namespace std;

extern const char* git_version_short;

int main() 
{
	cout << "Partikel " << git_version_short << endl;

	auto window = make_unique<GLWindow>("Partikel");

	return 0;	
}