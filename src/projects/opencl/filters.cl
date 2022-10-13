__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void hello()
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x == 0 && y == 0) printf("Hello OpenCL! I am [%d, %d]\n", x, y);
}

__kernel void boxFilter(read_only image2d_t srcImg, write_only image2d_t dstImg, int radius)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    int2 coord_O = (int2)(x, y);

    float sum = 0.0f;
    int num = (2 * radius + 1) * (2 * radius + 1);

    for (int r = y - radius; r <= y + radius; ++r)
    {
        for (int c = x - radius; c <= x + radius; ++c)
        {
            int2 coord = (int2)(c, r);
            uint4 pixel = read_imageui(srcImg, sampler, coord);
            sum += pixel.s0;

            //if (x == 0 && y == 0) printf("pixel[0,0]={%d,%d,%d,%d}\n", pixel.s0, pixel.s1, pixel.s2, pixel.s3);
        }
    }

    if (x == 0 && y == 0) printf("sum[0,0]=%f\n", sum);

    write_imageui(dstImg, coord_O, (uint4)(sum / num, 0, 0, 1));
}