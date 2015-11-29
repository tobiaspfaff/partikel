#include "compute/gpuSort.hpp"

using namespace std;

BitonicSort::BitonicSort(CLQueue& queue) :
	queue(queue),
	kernelSortLocal(queue, "BitonicSort_b.cl", "bitonicSortLocal"),
	kernelSortLocal1(queue, "BitonicSort_b.cl", "bitonicSortLocal1"),
	kernelMergeLocal(queue, "BitonicSort_b.cl", "bitonicMergeLocal"),
	kernelMergeGlobal(queue, "BitonicSort_b.cl", "bitonicMergeGlobal")
{
}

inline bool isPowerOf2(int x)
{
	return x != 0 && (x & (x - 1)) == 0;
}

//Note: logically shared with BitonicSort_b.cl!
static const unsigned int LOCAL_SIZE_LIMIT = 512U;

void BitonicSort::sort(const CLBuffer<cl_uint>& keySrc, CLBuffer<cl_uint>& keyDest,
	const CLBuffer<cl_uint>& dataSrc, CLBuffer<cl_uint>& dataDest, int N)
{
	if (N > keySrc.size || N > keyDest.size)
		fatalError("invalid size");
	unsigned dir = 1;
	if (N < LOCAL_SIZE_LIMIT || !isPowerOf2(N))
		fatalError("Can only sort 2^n arrays");

	if (N == LOCAL_SIZE_LIMIT)
	{
		kernelSortLocal.setArgs(keyDest.handle, dataDest.handle, keySrc.handle, dataSrc.handle, N, dir);		
		kernelSortLocal.enqueue(N / 2, LOCAL_SIZE_LIMIT / 2);
	}
	else
	{
		kernelSortLocal1.setArgs(keyDest.handle, dataDest.handle, keySrc.handle, dataSrc.handle);
		kernelSortLocal1.enqueue(N / 2, LOCAL_SIZE_LIMIT / 2);

		for (unsigned int size = 2 * LOCAL_SIZE_LIMIT; size <= N; size <<= 1)
		{
			for (unsigned stride = size / 2; stride > 0; stride >>= 1)
			{
				if (stride >= LOCAL_SIZE_LIMIT)
				{
					kernelMergeGlobal.setArgs(keyDest.handle, dataDest.handle, keyDest.handle, dataDest.handle, N, size, stride, dir);
					kernelMergeGlobal.enqueue(N / 2, LOCAL_SIZE_LIMIT / 4);					
				}
				else
				{
					kernelMergeLocal.setArgs(keyDest.handle, dataDest.handle, keyDest.handle, dataDest.handle, N, stride, size, dir);
					kernelMergeLocal.enqueue(N / 2, LOCAL_SIZE_LIMIT / 2);
				}
			}
		}
	}
}
