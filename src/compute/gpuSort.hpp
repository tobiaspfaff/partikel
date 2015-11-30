// OpenCL Bitonic sort

#ifndef COMPUTE_SORT_HPP
#define COMPUTE_SORT_HPP

#include "compute/computeMain.hpp"

class BitonicSort
{
public:
	BitonicSort(CLQueue& queue);

	void sort(const CLBuffer<cl_uint>& keySrc, CLBuffer<cl_uint>& keyDest,
			  const CLBuffer<cl_uint>& dataSrc, CLBuffer<cl_uint>& dataDest, int N);

protected:
	CLQueue& queue;
	CLKernel kernelSortLocal;
	CLKernel kernelSortLocal1;
	CLKernel kernelMergeLocal;
	CLKernel kernelMergeGlobal;
};

class RadixSort
{
public:
	RadixSort(CLQueue& queue);

	void sort(CLBuffer<cl_uint2>& data, int N);

protected:
	struct ConstData
	{
		int m_n;
		int m_nWGs;
		int m_startBit;
		int m_nBlocksPerWG;
	};

	enum
	{
		DATA_ALIGNMENT = 256,
		WG_SIZE = 64,
		BLOCK_SIZE = 256,
		ELEMENTS_PER_WORK_ITEM = (BLOCK_SIZE / WG_SIZE),
		BITS_PER_PASS = 4,
		NUM_BUCKET = (1 << BITS_PER_PASS),
		//	if you change this, change nPerWI in kernel as well
		NUM_WGS = 20 * 6,	//	cypress
		//			NUM_WGS = 24*6,	//	cayman
		//			NUM_WGS = 32*4,	//	nv
	};

	CLQueue& queue;
	CLKernel streamCountSortDataKernel;
	CLKernel prefixScanKernel;
	CLKernel sortAndScatterSortDataKernel;

	CLBuffer<cl_uint2> work;
	CLBuffer<cl_uint> workHisto;
};

#endif