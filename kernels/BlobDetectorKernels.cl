__kernel void mark_pixels_old(__global const uchar *in, __global int *out)
{
    // Get pixel BGR color
    int i = get_global_id(0);
    int j = i*3;

    int b = in[j + 0];
    int g = in[j + 1];
    int r = in[j + 2];

    // Convert to HSV
    int rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
    int rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
    int rgbDiff = rgbMax - rgbMin;

    int h;
    int s;
    int v = rgbMax;

    //out[i] = v;
    //return;

    if (v != 0)
    {
        s = 255 * rgbDiff / v;

        if (s == 0)
        {
            h = 0;
        } else if (rgbMax == r)
        {
            h = 0 + 43 * (g - b) / rgbDiff;
        } else if (rgbMax == g)
        {
            h = 85 + 43 * (b - r) / rgbDiff;
        } else
        {
            h = 171 + 43 * (r - g) / rgbDiff;
        }
    }

    // Light color
    if (v >= 200)
    {
        // Most likely yellow or orange
        if (h < 100)
        {
            out[i] = 1; // orange = 1
        }

            // Most likely white
        else
        {
            out[i] = 0; // white = 0
        }
    }

        // Most likely black, could be blue or green
    else if (v <= 50)
    {
        if (v >= 40 && s >= 175)
        {
            out[i] = 2; // blue = 2
        } else
        {
            out[i] = 3; // black = 3
        }
    }

        // Most likely blue
    else if (h >= 100 && h <= 130 && s >= 140)
    {
        out[i] = 2;
    }

        // Most likely green
    else
    {
        out[i] = 4;
    }
}

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
