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


RadixSort::RadixSort(CLQueue& queue) :
	queue(queue),
	streamCountSortDataKernel(queue, "RadixSort32Kernels.cl", "StreamCountSortDataKernel"),
	prefixScanKernel(queue, "RadixSort32Kernels.cl", "PrefixScanKernel"),
	sortAndScatterSortDataKernel(queue, "RadixSort32Kernels.cl", "SortAndScatterSortDataKernel"),
	work(queue), workHisto(queue)
{
	work.type = BufferType::Gpu;
	workHisto.type = BufferType::Gpu;
}


void RadixSort::sort(CLBuffer<cl_uint2>& data, int n)
{
	if (n % DATA_ALIGNMENT != 0)
		fatalError("Data size needs to be aligned");
	if (n > data.size)
		fatalError("Search size > array size");
	
	work.resize(n);
	CLBuffer<cl_uint2>* src = &data;
	CLBuffer<cl_uint2>* dst = &work;
	
	int minCap = NUM_BUCKET * NUM_WGS;
	workHisto.resize(minCap);
	
	//	ADLASSERT( ELEMENTS_PER_WORK_ITEM == 4 );
	const int sortBits = 32;
	assert(BITS_PER_PASS == 4);
	assert(WG_SIZE == 64);
	assert((sortBits & 0x3) == 0);
	
	int nWGs = NUM_WGS;
	ConstData cdata;
	{
		int blockSize = ELEMENTS_PER_WORK_ITEM*WG_SIZE;//set at 256
		int nBlocks = (n + blockSize - 1) / (blockSize);
		cdata.m_n = n;
		cdata.m_nWGs = NUM_WGS;
		cdata.m_startBit = 0;
		cdata.m_nBlocksPerWG = (nBlocks + cdata.m_nWGs - 1) / cdata.m_nWGs;
		if (nBlocks < NUM_WGS)
		{
			cdata.m_nBlocksPerWG = 1;
			nWGs = nBlocks;
		}
	}

	int count = 0;
	for (int ib = 0; ib < sortBits; ib += 4)
	{
		cdata.m_startBit = ib;
		streamCountSortDataKernel.call(NUM_WGS*WG_SIZE, WG_SIZE, *src, workHisto, cdata);

#ifdef __APPLE__
		fatalError("fast prefix scan not supported on OSX");
#endif		
		prefixScanKernel.call(128, 128, workHisto, cdata);
		
		//local sort and distribute
		sortAndScatterSortDataKernel.call(nWGs*WG_SIZE, WG_SIZE, *src, workHisto, *dst, cdata);
		
		swap(src, dst);
		count++;
	}

	if (count & 1)
	{
		assert(0); //need to copy from work to data
	}
}
