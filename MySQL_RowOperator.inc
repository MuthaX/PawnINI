#if(!defined mysql_included)
	#error Include file of the MySQL plugin have to be included before PawnTableHandler_Database.inc!
#endif
#if(!defined __PAWNINI_INCLUDED__)
	#error The file PawnINI.inc have to be included before PawnTableHandler_Database.inc!
#endif

//	Parsing SQL query results with use field names as indexes.
stock MySQL_Row_parse(
	#if !defined cache_get_field_type	// <=R39
	const db_handle,
	#endif
	dest_array[],
	const row_index,
	const dest_size,
	const ini_struct[][e_ini_struct_info],
	const n_parameters,
	const ini_name[]
) {
	#if !defined INIMODULE_IGNORE_STRUCT_CHECK
	if(
		INI_isInvalidStruct(ini_struct, n_parameters, ini_name) ||
		INI_isInvalidStruct_Dynamic(ini_struct, n_parameters, dest_size, ini_name)
	) {
		printf("[MySQL_Row_parse] cant read data from (\"%s\") due to errors in ini structure!", ini_name);
		return 0;
	}
	#else
	#pragma unused ini_name
	#pragma unused dest_size
	#endif
	new
		rows_number,
		parameter_idx,
		parameter_type,
		parameter_size,
		parameter_offset
	;
	#if(defined cache_get_field_type)
	cache_get_row_count(rows_number);
	#else
	rows_number = cache_get_row_count(db_handle);
	#endif
	if( row_index < rows_number ) {
		for(parameter_idx = 0; parameter_idx < n_parameters; ++parameter_idx) {
			parameter_type =	ini_struct[parameter_idx][e_type];
			parameter_offset =	ini_struct[parameter_idx][e_offset];

			if( parameter_type == 'i' || parameter_type == 'd' ) {
				#if(defined cache_get_field_type)
				cache_get_value_name_int(row_index, ini_struct[parameter_idx][e_name], dest_array[parameter_offset]);
				#else
				dest_array[parameter_offset] = cache_get_field_content_int(row_index, ini_struct[parameter_idx][e_name], db_handle);
				#endif
			} else if( parameter_type == 'f' ) {
				#if(defined cache_get_field_type)
				cache_get_value_name_float(row_index, ini_struct[parameter_idx][e_name], Float:dest_array[parameter_offset]);
				#else
				dest_array[parameter_offset] = _:cache_get_field_content_float(row_index, ini_struct[parameter_idx][e_name], db_handle);
				#endif
			} else if( parameter_type == 's' ) {
				parameter_size =		ini_struct[parameter_idx][e_size];
				#if(defined cache_get_field_type)
				cache_get_value_name(row_index, ini_struct[parameter_idx][e_name], dest_array[parameter_offset], parameter_size);
				#else
				cache_get_field_content(row_index, ini_struct[parameter_idx][e_name], dest_array[parameter_offset], db_handle, parameter_size);
				#endif
			}
		}
	}
	return 1;
}
