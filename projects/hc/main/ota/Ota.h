#pragma once

#include <string>
#include <stdint.h>

using namespace std;

namespace Ota
{
	bool CheckNeedUpdate(string fwVersion);
	int Update(string fwUrl, string fwChecksumAlgorithm, string fwChecksum);
	int Update(int fwSize, string fwChecksumAlgorithm, string fwChecksum);
	int UpdateChunk(int chunkId, uint8_t *data, int dataLen);
	int UpdateFinish();
}
