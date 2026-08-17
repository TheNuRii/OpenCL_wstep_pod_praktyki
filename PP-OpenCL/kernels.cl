#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

__kernel void PsnrPartialSum(__global const ushort* restrict RefLm, __global const ushort* restrict RefCb, __global const ushort* restrict RefCr,
                              __global const ushort* restrict TestLm, __global const ushort* restrict TestCb, __global const ushort* restrict TestCr,
                              __global       ulong*  restrict SquaredDiffLm, __global       ulong*  restrict SquaredDiffCb, __global       ulong*  restrict SquaredDiffCr,
                              const uint Width, const uint Height, const uint Margin, const uint Stride)
{
  // Trzyamy się konwencji dla tego stosujemy marginesy, które nie są potrzebne do wyliczenia
  const int x = get_global_id(0);
  const int y = get_global_id(1);
  if (x >= Width || y >= Height) { return; }
  const int Offset = ((Margin + y) * Stride) + Margin + x;

  // wyliczamy roznice i zapisujemy do bufora wynikowego jako kwadrat roznicy
  const float diffLm = (float)RefLm[Offset] - (float)TestLm[Offset];
  const float diffCb = (float)RefCb[Offset] - (float)TestCb[Offset];
  const float diffCr = (float)RefCr[Offset] - (float)TestCr[Offset];

  // kwadrat różnicy jest zawsze nieujemny — dopiero tu bezpiecznie rzutujemy na typ bez znaku
  SquaredDiffLm[y * Width + x] = (ulong)(diffLm * diffLm);
  SquaredDiffCb[y * Width + x] = (ulong)(diffCb * diffCb);
  SquaredDiffCr[y * Width + x] = (ulong)(diffCr * diffCr);
}

__kernel void PsnrReduceSum(__global const ulong* restrict SquaredDiffLm, __global const ulong* restrict SquaredDiffCb, __global const ulong* restrict SquaredDiffCr,
                             __global       ulong* restrict TotalDiffLm,   __global       ulong* restrict TotalDiffCb,   __global       ulong* restrict TotalDiffCr,
                             const uint Width, const uint Height)
{
  const int x = get_global_id(0);
  const int y = get_global_id(1);
  if (x >= Width || y >= Height) { return; }

  const ulong diffLm = SquaredDiffLm[y * Width + x];
  const ulong diffCb = SquaredDiffCb[y * Width + x];
  const ulong diffCr = SquaredDiffCr[y * Width + x];

  // atom_add (nie atomic_add) dla 64-bit — wymaga rozszerzenia zadeklarowanego na górze pliku
  atomic_add(TotalDiffLm, diffLm);
  atomic_add(TotalDiffCb, diffCb);
  atomic_add(TotalDiffCr, diffCr);
}

//===============================================================================================================================================================================================================
