#include "common.h"

using namespace std;
using namespace file_system;

int main(int argc, char const *argv[])
{
	BlockInfo block1;
	BlockInfo block2;

	if(block1==block2)
		cout << "compare 1: equal\n"; 
	else
		cout << "compare 1: not equal\n";
	
	block1.block_id_ = 1324;
	if(block1==block2)
		cout << "compare 2: equal\n"; 
	else
		cout << "compare 2: not equal\n";

	block2.block_id_ = 1324;
	if(block1==block2)
		cout << "compare 3: equal\n"; 
	else
		cout << "compare 3: not equal\n";

	return 0;
}