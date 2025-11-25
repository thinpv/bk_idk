#include <unistd.h>
// #include <dirent.h>
#include "Ota.h"
#include "Util.h"
#include "Log.h"
#include "Define.h"
#include <cstring>

bool Ota::CheckNeedUpdate(string fwVersion)
{
	// TODO: check current version
	return true;
}

int Ota::Update(string fwUrl, string fwChecksumAlgorithm, string fwChecksum)
{
	// // Update opkg
	// system("opkg update");
	// // sleep(30);

	// string filename = fwUrl.substr(fwUrl.find_last_of("/") + 1);
	// string extentsion = filename.substr(filename.find_last_of(".") + 1);
	// if (access(filename.c_str(), F_OK) != -1)
	// {
	// 	string checkSumCmd = Util::ExecuteCMD("rm " + filename);
	// }

	// // Download file
	// string wget = "wget " + fwUrl;
	// system(wget.c_str());
	// // TODO: check sleep time
	// // sleep(10);

	// if (fwChecksumAlgorithm == "sha256")
	// {
	// 	if (access(filename.c_str(), F_OK) != -1)
	// 	{
	// 		string checkSumCmd = Util::ExecuteCMD("sha256sum " + filename);
	// 		string checkSum = checkSumCmd.substr(0, checkSumCmd.find(" "));
	// 		LOGD("checksumGet: %s,checksumCal: %s", checkSum.c_str(), fwChecksum.c_str());
	// 		if (checkSum != fwChecksum)
	// 		{
	// 			return CODE_OTA_CHECK_CRC_ERROR;
	// 		}
	// 	}
	// }

	// string tar = "tar -xJf " + filename;
	// string fileTar = filename.substr(0, filename.find(".tar.xz"));
	// system(tar.c_str());
	// if (access(fileTar.c_str(), F_OK) != -1)
	// {
	// 	// Kiem tra co ton tai file .ipk trong folder
	// 	string dir_path = "/root/" + fileTar;
	// 	DIR *dir = opendir(dir_path.c_str());
	// 	if (dir)
	// 	{
	// 		struct dirent *entry;
	// 		while ((entry = readdir(dir)) != NULL)
	// 		{
	// 			if (entry->d_type == DT_REG && strcmp(strrchr(entry->d_name, '.'), ".ipk") == 0)
	// 			{
	// 				string install = "/usr/bin/nohup /bin/opkg install " + dir_path + "/" + string(entry->d_name) + "> /log.txt 2>&1 &";
	// 				cout << install << endl;

	// 				system(install.c_str());
	// 			}
	// 		}
	// 		closedir(dir);
	// 	}
	// 	else
	// 	{
	// 		LOGW("File ipk not found");
	// 		return CODE_ERROR;
	// 	}
	// }
	// else
	// {
	// 	LOGW("TAR file error");
	// 	return CODE_ERROR;
	// }
	return CODE_OK;
}

int Ota::Update(int fwSize, string fwChecksumAlgorithm, string fwChecksum)
{
	LOGW("This OTA method is not supported");
	return CODE_ERROR;
}

int Ota::UpdateChunk(int chunkId, uint8_t *data, int dataLen)
{
	LOGW("This OTA method is not supported");
	return CODE_ERROR;
}

int Ota::UpdateFinish()
{
	LOGW("This OTA method is not supported");
	return CODE_ERROR;
}
