__kernel void SSDPartialSum(__global const ushort* restrict Ref, __global const ushort* restrict Test,
                             __global       ulong*  restrict SquaredDiff, const uint Width)
{
  const uint y = get_global_id(0);

  ulong rowSum = 0;
  for (uint i = 0; i < Width; i++) {
    const uint offset = y * Width + i;
    const int  diff    = (int)Ref[offset] - (int)Test[offset];
    rowSum += (ulong)(diff * diff);
  }

  SquaredDiff[y] = rowSum;
}

__kernel void SSDReduceSum(__global const ulong* restrict SquaredDiff, __global       ulong* restrict TotalDiff,
                           const uint NumElements)
{
  ulong local_sum = 0;
  for (uint i = 0; i < NumElements; i++) {
    local_sum += SquaredDiff[i];
  }
  
  TotalDiff[0] = local_sum;
}

//===============================================================================================================================================================================================================
