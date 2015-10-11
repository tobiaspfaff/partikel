#include <iostream>

using namespace std;

extern const char* git_version_short;

int main() {
	cout << "Partikel " << git_version_short << endl;

	return 0;	
}