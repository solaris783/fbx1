//
// Entry point for app
//

// sytem includes
#include <process.h> // thread library ('_beginthread')
#include <string>
#include <vector>


// Basic FBX init stuff and includes
#include "common.h"
#include "fbxdefs.h"
#include "ProcessContent.h"
#include "Weld.h"
#include "PerformanceCounter.h"




//////////////////////////////////////////
// NAMESPACE
//////////////////////////////////////////
using namespace std;	// to avoid having to write std:: every time I want to use a standard library func



//////////////////////////////////////////
// PROTOTYPES
//////////////////////////////////////////
void ProcessFbxFileThreadStart(void *pData);



//////////////////////////////////////////
// GLOBALS
//////////////////////////////////////////
FbxLib G_fbxLib = {NULL, NULL};
FbxLibAndFilename G_fbxInfo[MAX_FILE_COUNT];
int G_threadCnt;
bool G_bVerbose = false;

namespace std
{
	template <> struct hash<Vec3>
	{
		size_t operator()(Vec3 v)
		{
			const unsigned int * h = (const unsigned int *)(&v);
			unsigned int f = (h[0]+h[1]*11-(h[2]*17))&0x7fffffff;     // avoid problems with +-0
			return (f>>22)^(f>>12)^(f);
		}
	};
}



//////////////////////////////////////////
//
// MAIN
//
//////////////////////////////////////////
int main(int argc, char** argv)
{
	printf("Processing fbx file list...\n");

	int stArg = 1; // first filename in argv to process according to usage

	// verbose mode?  If so, first filename in argv according to usage will be 2
	if(argc >= 2)
	{
		string arg1(argv[1]);
		size_t loc = arg1.find("-v");
		if(loc != string::npos)
		{
			printf("\tVerbose mode On...\n");
			G_bVerbose = true;
			stArg = 2;
		}
	}

	if(argc < stArg +1)
	{
		printf("Usage: fbx1.exe [ -v] <filename1.fbx> <filename2.fbx> ...\n");
		return 0;
	}

    // Prepare the FBX SDK.
	InitializeSdkObjects(G_fbxLib.lSdkManager, G_fbxLib.lScene);

	// keep track of how many thread we make so we can track errors
	G_threadCnt = 0;

	for(int i=stArg; i<argc; i++)
	{
		G_fbxInfo[i].pFbxLib = &G_fbxLib;
		G_fbxInfo[i].fileName = argv[i];
		uintptr_t ret;

		// kick off thread
		ret =_beginthread(ProcessFbxFileThreadStart, 0, (void *) &G_fbxInfo[i]);

		// check for errors
		if(ret > 0)
		{
			G_threadCnt++;
		}
		else
		{
			printf("***   Error in main.cpp creating a thread for %s (arg #%d)\n", argv[i], i);
		}
	}

	// wait for all threads to end
	if(G_bVerbose)
		printf("\tWaiting for all files to finish being processed...\n");
	
	while(G_threadCnt > 0);

	printf("Done.\n");

	return 0;
}



//////////////////////////////////////////
//
// Entry point for thread.  One thread per file processed
//
//////////////////////////////////////////
void ProcessFbxFileThreadStart(void *pData)
{
	FbxLibAndFilename *pFbxInfo = static_cast<FbxLibAndFilename *>(pData);

	if(G_bVerbose)
		printf("\t\tProcessing: %s...\n", pFbxInfo->fileName.c_str());

	bool lResult = LoadScene(pFbxInfo->pFbxLib->lSdkManager, pFbxInfo->pFbxLib->lScene, pFbxInfo->fileName.c_str());

    if(lResult == false)
    {
		FBXSDK_printf("***  An error occurred while loading the scene \"%s\" (main.cpp->ProcessFbxFile->LoadScene)...\n", pFbxInfo->fileName.c_str());
    }
	else
	{
		ProcessContent proc(pFbxInfo->fileName);	// create the data structure that will hold all of the file's data
		proc.Start(pFbxInfo->pFbxLib->lScene);		// process the file (extract all data)
	}

	G_threadCnt--; //done update global counter
}
