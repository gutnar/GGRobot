__kernel void mark_pixels(__global const uchar *in, __global int *centroids, const int numberOfCentroids, __global int *out)
{
    // Get pixel BGR color
    int i = get_global_id(0);
    int j = i*3;

    int b = in[j + 0];
    int g = in[j + 1];
    int r = in[j + 2];

    // Convert to HSV
    /*
    int rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
    int rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
    int rgbDiff = rgbMax - rgbMin;

    int h;
    int s;
    int v = rgbMax;*/

    out[i] = 0;

    for (int k = 0; k < numberOfCentroids; ++k) {
        int l = 9*k;
        if (b >= centroids[l] && g >= centroids[l + 1] && r >= centroids[l + 2] && b <= centroids[l + 3] && g <= centroids[l + 4] && r <= centroids[l + 5]) {
            out[i] = k + 1;
        }
    }
}

__kernel void closest_centroids(__global const uchar *in, __global int *centroids, const int numberOfCentroids, __global int *out)
{
    // Get pixel BGR color
    int i = get_global_id(0);
    int j = i*3;

    int b = in[j + 0];
    int g = in[j + 1];
    int r = in[j + 2];

    out[i] = 0;
    float minDist = 0;

    for (int k = 0; k < numberOfCentroids; ++k) {
        int l = 9*k;

        if (b >= centroids[l] && g >= centroids[l + 1] && r >= centroids[l + 2] && b <= centroids[l + 3] && g <= centroids[l + 4] && r <= centroids[l + 5]) {
            float dist = (b - centroids[l+6])*(b - centroids[l+6]) + (g - centroids[l+7])*(g - centroids[l+7]) + (r - centroids[l+8])*(r - centroids[l+8]);

            if (out[i] == 0 || dist < minDist) {
                out[i] = k + 1;
                minDist = dist;
            }
        }
    }
}
