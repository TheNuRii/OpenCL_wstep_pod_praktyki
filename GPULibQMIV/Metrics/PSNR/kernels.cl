#define uint16_t ushort
#define uint32_t uint
#define uint64_t ulong

__kernel void SSDPartialSum(__global const uint16_t* restrict Ref, __global const uint16_t* restrict Test,
                             __global       uint64_t*  restrict SquaredDiff, 
                            const uint32_t Width, const uint32_t Height, const uint32_t Margin, const uint32_t Stride)
{
  const uint y = get_global_id(0);
  if (y >= Height) { return; }

  uint64_t rowSum = 0;
  uint offset_y = (y + Margin)*Stride;
  
  for (size_t i = 0; i < Width; i++) {
    const uint offset = offset_y + (Margin + i);
    const int  diff    = (int)Ref[offset] - (int)Test[offset];
    rowSum += (ulong)(diff * diff);
  }

  SquaredDiff[y] = rowSum;
}

__kernel void SSDReductionSum(__global const uint64_t* SquaredDiff, __global       uint64_t* ReductionSum, const uint32_t Height)
{
  uint64_t rowSum = 0;
  for (size_t i = 0 ; i < Height; i++){
    rowSum += SquaredDiff[i];
  } 
  
  *ReductionSum = rowSum;
}

//==============================================================================================================================================================================================================gl