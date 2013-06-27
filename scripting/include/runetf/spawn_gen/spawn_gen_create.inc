

//// Temporary RuneGen
new t_Array[RuneGen];
const RUNE_BLOCK_SIZE = sizeof(t_Array)

ResetTempGen(local_Array[RuneGen])
{
	local_Array[Id] = 0;
	local_Array[g_ori] = 0.0;
	local_Array[_:g_ori + 1] = 0.0;
	local_Array[_:g_ori + 2] = 0.0;
	local_Array[g_ang] = 0.0;
	local_Array[_:g_ang + 1] = 0.0;
	local_Array[_:g_ang + 2] = 0.0;
	local_Array[g_force] = 0.0;
	local_Array[g_StartDisabled] = false;
}

SetTempGenVec(local_Array[RuneGen], idx, Float:vec[3])
{
	local_Array[_:idx] 			= _:vec[0];
	local_Array[_:idx + 1] 	= _:vec[1];
	local_Array[_:idx + 2] = 	_:vec[2];
}

SetTempGenInt(local_Array[RuneGen], idx, _:value)
{
	local_Array[_:idx] 				= value;
}

GetTempGenVec(local_Array[], idx, Float:vec[3])
{
	vec[0] = Float:local_Array[_:idx];
	vec[1] = Float:local_Array[_:idx + 1];
	vec[2] = Float:local_Array[_:idx + 2];
}

GetTempGenInt(local_Array[], idx, &value)
{
	value = local_Array[idx];
}


get_id_info(const Handle:array, index, &_id = 0)
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	GetTempGenInt( pack , _:Id, _id);
	return _id;
}

get_origin_info(const Handle:array, index, Float:vec[3])
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	GetTempGenVec(pack, _:g_ori, vec);
}

get_angle_info(const Handle:array, index, Float:vec[3])
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	GetTempGenVec(pack, _:g_ang, vec);
}

get_force_info(const Handle:array, index, &Float:_force = 0)
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	GetTempGenInt( pack , _:g_force, _force);
	return _force;
}

get_disable_info(const Handle:array, index, &_disabled = 0)
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	GetTempGenInt( pack , _:g_StartDisabled, _disabled);
	return _disabled;
}


set_id_info(&Handle:array, index, _id)
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	SetTempGenInt( pack, _:Id, _id);
	SetArrayArray(array, index, pack, RUNE_BLOCK_SIZE);
}

set_origin_info(&Handle:array, index, Float:vec[3])
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	SetTempGenVec( pack, _:g_ori, vec);
	SetArrayArray(array, index, pack, RUNE_BLOCK_SIZE);
}

set_angle_info(&Handle:array, index, Float:vec[3])
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	SetTempGenVec( pack, _:g_ang, vec);
	SetArrayArray(array, index, pack, RUNE_BLOCK_SIZE);
}

set_force_info(&Handle:array, index, &_force = 0)
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	SetTempGenInt( pack, _:g_force, _force);
	SetArrayArray(array, index, pack, RUNE_BLOCK_SIZE);
}


set_disable_info(&Handle:array, index, _disabled)
{
	new pack[RuneGen];
	GetArrayArray(array,index, pack, RUNE_BLOCK_SIZE);
	SetTempGenInt( pack , _:g_StartDisabled, _disabled);
	SetArrayArray(array,index,pack,RUNE_BLOCK_SIZE);
}



add_rune_info(&Handle:array, rune_info[], bool:append=true)
{
	if(array == INVALID_HANDLE)
		array = CreateArray(RUNE_BLOCK_SIZE);

	if(append)
		return PushArrayArray(array, rune_info, RUNE_BLOCK_SIZE);

	ShiftArrayUp(array,0);
	SetArrayArray(array, 0, rune_info, RUNE_BLOCK_SIZE);
	return 0;
}


