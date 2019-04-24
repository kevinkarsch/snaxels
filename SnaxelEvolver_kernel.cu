// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// includes, kernels
#include <cuda.h>
#include <cutil_math.h>
#include <cutil_inline.h>

__global__ void findInitVertsKernel(float3 *vertPositions_d,
									int numVerts,
									int *nValenceVerts_d,
									int *idxValenceVerts_d,
									float3 *valenceEdgeNormals_d,
									float3 viewpoint,
									bool *initIndices_d)
{
	int idx = threadIdx.x + blockIdx.x * blockDim.x;
	if(idx >= numVerts || !initIndices_d[idx]) return;
	float3 vv = normalize(vertPositions_d[idx] - viewpoint);
	//Go through two edges at a time
	int size = nValenceVerts_d[idx]-1;
	for(int i = 0; i < size; i++) {
		// Check if the view vector lies inside of these normals on the gauss map sphere
		float3 polyNormal = normalize( cross(valenceEdgeNormals_d[i+idxValenceVerts_d[idx]],
								             valenceEdgeNormals_d[i+1+idxValenceVerts_d[idx]]) );
		if(dot(vv,valenceEdgeNormals_d[i+idxValenceVerts_d[idx]]) > 0 || dot(vv, polyNormal) > 0) {
			initIndices_d[idx] = false;
			return;
		}
	}
}

extern "C" void findInitVerts(float3 *vertPositions_d,
								int numVerts,
								int *nValenceVerts_d,
								int *idxValenceVerts_d,
								float3 *valenceEdgeNormals_d,
								float3 viewpoint,
								bool *initIndices_d,
								bool *initIndices) 
{
	//Set kernel params
	int blocksPerGrid = 1024;
	int threadsPerBlock = int(ceil(float(numVerts)/float(blocksPerGrid)));
	dim3 grids(blocksPerGrid, 1, 1);
	dim3 blocks(threadsPerBlock, 1, 1);
	//Run kernel
	findInitVertsKernel <<< grids, blocks >>> (vertPositions_d, numVerts, 
			nValenceVerts_d, idxValenceVerts_d, valenceEdgeNormals_d, 
			viewpoint, initIndices_d);
	//cudaThreadsSyncronize();
	//Copy device memory back to host
    cutilSafeCall(cudaMemcpy(initIndices, initIndices_d, numVerts*sizeof(bool), cudaMemcpyDeviceToHost));
}
