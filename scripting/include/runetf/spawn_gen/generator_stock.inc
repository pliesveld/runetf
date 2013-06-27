#if defined __util_generator_stock__
	#endinput
#endif

#define __util_generator_stock__

stock IsValidIter(&Handle:array, array_idx)
{
  new size;
  return array != INVALID_HANDLE && (size = GetArraySize(array)) > 0 && array_idx >= 0 && array_idx < size;
}


