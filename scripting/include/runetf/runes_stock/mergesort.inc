#if defined __mergesort_cell__
	#endinput
#endif
#define __mergesort_cell__


/* usage:
	Sorts an array of cells


#include "spawn_gen/spawn_gen_util.inc"
`new Handle:t_array = INVALID_HANDLE;

public OnPluginStart()
{
	t_array = CreateArray(ARRAY_BLOCK_SIZE);
	for(new i = 0; i < 32;i++)
	{
		PushArrayCell(t_array,GetURandomInt()%400);
	}
}

public OnPluginEnd()
{
	PrintArray(t_array);
	t_array = merge_sort(t_array);
	PrintArray(t_array);
}



*/

static ARRAY_BLOCK_SIZE = 1;

stock add_val(i, &Handle:l)
{
	if(l == INVALID_HANDLE)
		l = CreateArray(ARRAY_BLOCK_SIZE);
	PushArrayCell(l,i);
}

stock Handle:merge_sort(&Handle:m)
{
	new len;
	if( (len = GetArraySize(m)) <= 1)
		return m;

	new Handle:left = INVALID_HANDLE, Handle:right = INVALID_HANDLE;
	new middle = GetArraySize(m) / 2;

	new i;
	for(i = 0; i < middle; ++i)
		add_val(GetArrayCell(m,i), left);
	for(i = middle; i < len; ++i)
		add_val(GetArrayCell(m,i), right);
	CloseHandle(m);

	left = merge_sort(left);
	right = merge_sort(right);

	return merge(left,right);
}

stock AppendFirst(&Handle:a_src, &Handle:a_dest)
{
	new v = GetArrayCell(a_src, 0);
	RemoveFromArray(a_src,0);
	PushArrayCell(a_dest, v);
}

stock Handle:merge(&Handle:left,&Handle:right)
{
	new left_size = GetArraySize(left), right_size = GetArraySize(right);
	new Handle:result = CreateArray(ARRAY_BLOCK_SIZE, left_size+right_size);

	for(;
		(left_size > 0 || right_size > 0);
		left_size = GetArraySize(left),right_size = GetArraySize(right))
	{
		if( left_size > 0 && right_size > 0 )
		{
			if( GetArrayCell(left, 0) > GetArrayCell(right,0) )
			{
				AppendFirst(right, result);
			} else {
				AppendFirst(left, result);
			}
		} else if( left_size > 0 ) {
			AppendFirst(left, result);
		} else if( right_size > 0) {
			AppendFirst(right, result);
		}
	}

	CloseHandle(left);
	CloseHandle(right);

	return result;
}


