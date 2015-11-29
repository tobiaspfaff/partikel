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

#endif