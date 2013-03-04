////////////////////////////////////////////////
// WELD.H
//
// This file contains the function for welding
// vertex data so that we can remove duplicate
// vertex positions, normals, etc.
// It is a template so that it can apply to all
// our data types.
////////////////////////////////////////////////


#ifndef _WELD_H_
#define _WELD_H_



/*----------------------------------------------------------------------------
	Headers
----------------------------------------------------------------------------*/

#include <vector>		// vector<T>
#include <functional>	// equal_to<T>



/*----------------------------------------------------------------------------
	Functions:
----------------------------------------------------------------------------*/

// as the name implies
inline size_t NextPowerOfTwo(size_t x)
{
	size_t p = 1;
	while( x > p )
	{
		p += p;
	}
	return p;
}



#if 0
// In case hash_map is not provided.
//namespace std
//{
	template <class Key> struct myhash {};
	
	template <> struct myhash<int>
	{
		size_t operator()(int x) { return x; }
	};
	
	template <> struct myhash<const char *> 
	{
		size_t operator()(const char * str)
		{
			size_t h = 5381;
			while (*str)
			{
				h = ((h << 5) + h) ^ *str;
				str++;
			}
			return h;
		}
	};

	template <> struct myhash<Vec3>
	{
		size_t operator()(Vec3 v)
		{
			const unsigned int * h = (const unsigned int *)(&v);
			unsigned int f = (h[0]+h[1]*11-(h[2]*17))&0x7fffffff;     // avoid problems with +-0
			return (f>>22)^(f>>12)^(f);
		}
	};
//}
#endif




/** Generic welding routine. This function welds the elements of the vector p
 * and returns the cross references in the xrefs array. To compare the elements
 * it uses the standard hash and key_equal functors.
 *
 * This code is based on the ideas of Ville Miettinen and Pierre Terdiman.
 */
template <class T, class HashFunction, class BinaryPredicate>
size_t weld( std::vector<T> & p, std::vector<size_t> & xrefs, HashFunction hash, BinaryPredicate equal )
{
	size_t const NIL = size_t(~0);							// linked list terminator symbol.
	size_t const N = p.size();								// # of input vertices.
	size_t outputCount = 0;									// # of output vertices
	size_t hashSize = NextPowerOfTwo(N);					// size of the hash table
	size_t * const hashTable = new size_t[hashSize + N];	// hash table + linked list
	size_t * const next = hashTable + hashSize;				// use bottom part as linked list

	memset( hashTable, NIL, hashSize*sizeof(size_t) );		// init hash table (NIL = 0xFFFFFFFF so memset works)

	// xrefs and p have the same size.
	xrefs.resize(N);

	for (size_t i = 0; i < N; ++i)
	{

		const T & e = p[i];
		size_t hashValue = hash(e) & (hashSize-1);
		size_t offset = hashTable[hashValue];

		// traverse linked list
		while( offset != NIL && !equal(p[offset], e) )
		{
			offset = next[offset];
		}

		xrefs[i] = offset;

		// no match found - copy vertex & add to hash
		if( offset == NIL )
		{
			// save xref
			xrefs[i] = outputCount;

			// copy vertex
			p[outputCount] = e;

			// link to hash table
			next[outputCount] = hashTable[hashValue];

			// update hash heads and increase output counter
			hashTable[hashValue] = outputCount++;
		}
	}

	// cleanup
	delete [] hashTable;

	// drop duplicates.
	p.resize(outputCount);

	// number of output vertices
	return outputCount;
}





// example code
#if 0
{
	TimerPerformanceCounter mytimer;
	TimerPerformanceCounter mytimer2;

	vector<Vec3> vec_vector;
#if 0
	vec_vector.push_back( Vec3(0.0f, 1.0f, 0.0f) );
	vec_vector.push_back( Vec3(1.0f, 1.0f, 0.0f) );
	vec_vector.push_back( Vec3(0.0f, 1.0f, 1.0f) );
	vec_vector.push_back( Vec3(0.0f, 1.0f, 0.0f) );
	vec_vector.push_back( Vec3(0.0f, 1.0f, 1.0f) );
	vec_vector.push_back( Vec3(1.0f, 1.0f, 1.0f) );
	vec_vector.push_back( Vec3(1.0f, 1.0f, 0.0f) );
#endif
	mytimer2.Start();

	for(int i=0; i < 1000000; i++)
	{
		Vec3 vec;
		vec.x = IN_CutPrecision(rand_FloatRange(-10.0f, 10.0f), 1);
		vec.y = IN_CutPrecision(rand_FloatRange(-10.0f, 10.0f), 1);
		vec.z = IN_CutPrecision(rand_FloatRange(-10.0f, 10.0f), 1);
		vec_vector.push_back(vec);
	}
	mytimer2.Stop();
	unsigned __int64 interval2 = mytimer2.Interval();


	mytimer.Start();
	vector<size_t> xrefs;
	int num = weld(vec_vector, xrefs, std::hash<Vec3>(), std::equal_to<Vec3>());
	mytimer.Stop();
	unsigned __int64 interval = mytimer.Interval();
	int a;
	a = 1;
#endif




/** Reorder the given array accoding to the indices given in xrefs.
 * Use this after weld to reorder an array according to its result:
 * @code
 * size_t num = weld(points, xrefs);
 * reorder(texcoords, num, xrefs);
 * @endcode
 */
template <class T>
void reorder(std::vector<T> & array, const std::vector<size_t> & xrefs, const size_t num)
{
	std::vector<T> new_array;
	new_array.resize(num);

	for(size_t i = 0; i < num; ++i) {
		new_array[i] = array[xrefs[i]];
	}

	// replace old array by the new one.
	std::swap(array, new_array);
}



#endif // _WELD_H_
