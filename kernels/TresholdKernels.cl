__kernel void
findClosestCentroid(__global const int *pixels, __global const float *centroids, const int numberOfCentroids,
                    __global int *clusterMap)
{
    int p = get_global_id(0);

    int o = 3 * p;
    int b = pixels[o + 0];
    int g = pixels[o + 1];
    int r = pixels[o + 2];

    // Find closest centroid
    float minDistance = -1;
    int closestCentroidIndex = -1;

    for (int i = 0; i < numberOfCentroids; ++i)
    {
        int j = 3 * i;
        float distance = (b - centroids[j + 0]) * (b - centroids[j + 0]) +
                         (g - centroids[j + 1]) * (g - centroids[j + 1]) +
                         (r - centroids[j + 2]) * (r - centroids[j + 2]);

        if (distance < minDistance || minDistance == -1)
        {
            minDistance = distance;
            closestCentroidIndex = i;
        }
    }

    clusterMap[p] = closestCentroidIndex;
}