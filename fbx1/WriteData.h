//
// Write all extracted data out
//


#ifndef __WRITE_DATA__H
#define __WRITE_DATA__H



//
// System headers
//
#include <assert.h>




//
// Project headers
//
#include "DataTypes.h"



///////////////////////////////////////////////////////
// CLASSES
//
// This class maintains the file saving information
// and saves data in the appropriate slot.
///////////////////////////////////////////////////////
class WriteData
{
	public:
		WriteData();
		void SetFilename(string input_filename);
		void RecordVertCoord(Vec3 *pVert);

	private:
		FileData m_fileData;
		string m_outputFilename;
};





#endif
