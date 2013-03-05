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

		///////////////////////////////////////////////
		// Functions for recording vertex components
		void RecordVertCoord(Vec3 *pVert);
		void RecordVertColor(ColorRGBA *pRgba);
		void RecordVertTexCoord(TexCoord *pTexCoord);
		void RecordVertNormal(Vec3 *pNorm);

	private:
		FileData m_fileData;
		string m_outputFilename;
};





#endif
