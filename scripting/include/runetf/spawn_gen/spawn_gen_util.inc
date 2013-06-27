#if defined __spawn_gen_util__
	#endinput
#endif
#define __spawn_gen_util__


stock CloseArray(&Handle:array)
{
  if(array == INVALID_HANDLE)
    return;
//  ClearArray(array);
  CloseHandle(array);
  array = INVALID_HANDLE;
}

stock ArrayToString(&Handle:a_array, String:buf[], len, &i = 0)
{
	new b_written = strlen(buf);
	for(; i < GetArraySize(a_array);++i)
	{
		decl String:t_id[8];
		new b_towrite = Format(t_id,sizeof(t_id), "%3d ", GetArrayCell(a_array, i));
		if(b_towrite + b_written > len)
			return i;
		b_written += StrCat(buf, len, t_id);
	}

	return i;
}

stock PrintArray(&Handle:a_array, client=0)
{
	decl String:buf[128];
	new array_size = GetArraySize(a_array);
	new cnt;
	buf[0] = '\0';
	//Format(buf, sizeof(buf), "size %d: ", array_size);
	do {
		ArrayToString(a_array, buf, sizeof(buf), cnt);
		PrintToConsole(client,buf);
		buf[0] = '\0';
	} while( cnt < array_size);
	//LogMessage(buf);
}


