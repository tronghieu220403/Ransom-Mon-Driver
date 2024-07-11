#include "common.h"

#include <vector>

using namespace std;

class Ioctl
{
private:
	HANDLE device_ = INVALID_HANDLE_VALUE;
public:
	bool Create();

	vector<unsigned char> StartMonitor();

	vector<unsigned char> StopMonitor();

	void Close();
};