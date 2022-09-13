#include <myCV/myCV.h>
#include <myUtils/myUtils.h>
#include <vector>
#include <algorithm>

int boxFilter(cv::Mat& dstImg, cv::Mat& srcImg, int& radius)
{
	int err = 0;

	if (radius < 0 ||
		dstImg.rows != srcImg.rows ||
		dstImg.cols != srcImg.cols ||
		dstImg.channels() != srcImg.channels()) 
	{
		CHECK_RET(-1, "Unsupported args");
	}

	int width = dstImg.cols;
	int hight = dstImg.rows;
	int depth = dstImg.channels();
	float num = (2 * radius + 1) * (2 * radius + 1);
	float inv = 1.0f / num;

	for (int r = 0; r < hight; ++r) 
	{
		for (int c = 0; c < width; ++c) 
		{
			std::vector<int> sum(depth, 0);
			uint8_t* pDst = dstImg.ptr<uint8_t>(r);
			for (int rr = -radius; rr <= radius; ++rr) 
			{
				int row = r + rr;
				if (row < 0) row = 0;
				if (row >= hight) row = hight - 1;
				uint8_t* pSrc = srcImg.ptr<uint8_t>(row);
				for (int cc = -radius; cc <= radius; ++cc) 
				{
					int col = c + cc;
					if (col < 0) col = 0;
					if (col >= width) col = width - 1;
					for (int d = 0; d < depth; ++d) 
					{
						sum[d] += pSrc[col * depth + d];
					}
				}
			}
			for (int d = 0; d < depth; ++d)
			{
				float val = sum[d] * inv + 0.5f;
				pDst[c * depth + d] = std::min(val, 255.0f);
			}
		}
	}

	return err;
}