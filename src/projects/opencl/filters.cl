__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void hello()
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    printf("Hello OpenCL! I am [%d, %d]\n", x, y);
}

__kernel void boxFilter(read_only image2d_t srcImg, write_only image2d_t dstImg)
{
    int c = get_global_id(0);
    int r = get_global_id(1);

    int2 coord = (int2)(c, r);
    uint4 pixel = read_imageui(srcImg, sampler, coord);

    if (c == 0 && r == 0) {
        printf("[blur3x3] pixel[%d, %d] = (%d, %d, %d, %d)\n", c, r, pixel.s0, pixel.s1, pixel.s2, pixel.s3);
    }

    pixel.s0 = 255 - pixel.s0;

    write_imageui(dstImg, coord, pixel);
}