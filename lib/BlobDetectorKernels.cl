__kernel void mark_pixels(__global const int *in, __global int *out)
{
    // Get pixel BGR color
    int x = get_global_id(0);
    int y = get_global_id(1);
    int i = y * 640 + x;
    int j = i * 3;

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