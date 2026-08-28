#define uint16_t ushort
#define uint32_t uint
#define uint64_t ulong
#define internal_float float


__kernel void ProcesBlock(__global const uint16_t* restrict BuffRef, __global const uint16_t* restrict BuffTest,
                          __global       internal_float*     restrict BuffProcesBlock, const uint32_t BlockSize,
                          const uint32_t Width, const uint32_t Height, const uint32_t Margin, uint32_t Stride,
                          const internal_float C1, const internal_float C2) 
{
    
    const uint32_t X = get_global_id(0);
    const uint32_t Y = get_global_id(1);
    if (X >= Width || Y >= Height) { return; }

    uint64_t sumX      = 0;
    uint64_t sumY      = 0;
    uint64_t sumX2     = 0;
    uint64_t sumY2     = 0;
    uint64_t sumXY     = 0;


    const uint32_t N      = BlockSize * BlockSize;
    const uint32_t Radius = BlockSize / 2;
    // Po stronie hosta jest wykrywane Margin < Radius case
    const uint32_t StartX =  (int)X + (int)Margin - (int)Radius;
    const uint32_t StartY = (int)Y + (int)Margin - (int)Radius;

    for (uint32_t row = 0; row < BlockSize; row++)
    {
        const __global uint16_t* XRow = 
        BuffRef + StartY * Stride + StartX + row * Stride;
 
        const __global uint16_t* YRow = 
        BuffTest + StartY * Stride + StartX + row * Stride;

        for (uint32_t col = 0; col < BlockSize; col++)
        {
            const uint64_t x = XRow[col];
            const uint64_t y = YRow[col];

            sumX += x;
            sumY += y;

            sumX2 += x * x;
            sumY2 += y * y;

            sumXY += x * y;
        }
    }

    const internal_float meanX  = (internal_float)(sumX) / N;
    const internal_float meanY  = (internal_float)(sumY) / N;
    const internal_float varX = ((internal_float)sumX2 -
     (internal_float)sumX * meanX) / (N - 1);

    const internal_float varY = ((internal_float)sumY2 -
     (internal_float)sumY * meanY) / (N - 1);

    const internal_float covXY = ((internal_float)sumXY -
     (internal_float)sumX * meanY) / (N - 1);
    
    const internal_float numerator = (2.0f * meanX * meanY + C1) *
        (2.0f * covXY + C2);

    const internal_float denominator = (meanX * meanX + meanY * meanY + C1) *
        (varX + varY + C2);

    const uint32_t offset = Y * Width + X;

    BuffProcesBlock[offset] = numerator / denominator;
}

__kernel void ProcesLine(__global const internal_float* restrict BuffBlockLine, __global internal_float* restrict BuffProcesLine,
                        const uint32_t Width, const uint32_t Height, const uint32_t Margin, uint32_t Stride)
{
    const uint32_t y = get_global_id(0);
    if (y >= Height) { return; }

    internal_float      rowSum = 0;
    const uint32_t offset_y = y * Width;

    for (uint32_t x = 0; x < Width; x++) {
        rowSum += BuffBlockLine[offset_y + x];
    }

    BuffProcesLine[y] = rowSum;
}

__kernel void ReduceSum(__global const internal_float* BuffProcesLine, __global internal_float* BuffReductionSum,
                        const uint32_t Height) 
{
    internal_float colSum = 0;
    for (uint32_t i = 0; i < Height; i++) {
        colSum += BuffProcesLine[i];
    }

    *BuffReductionSum = colSum; 
}