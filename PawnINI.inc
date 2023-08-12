/**************************************************************
	You can define next macros (before include this file):
	* To ignore any validation (Best use on production server & Never define this for development/debugging server):
		#define INIMODULE_IGNORE_STRUCT_CHECK
	* To make ini-structure errors non fatal (no gamemode shutdown):
		#define INIMODULE_ERRORSTRUCT_NONFATAL
	* Ta silent runtime reading warnings:
		#define INIMODULE_SILENT_RUNTIME_WARNS

	Example use:
		enum e_dest {
			E_INT,
			E_FLT,
			E_INTARR[4],
			E_FLTARR[4],
			E_STRING[16]
		};
		new dest_arr[e_dest];

		new const struct_v[][e_ini_struct_info] = {
			{'i',	e_dest:E_INT,		1,	0,	3,	"int"},
			{'i',	e_dest:E_INTARR,	4,	0,	6,	"intarr"},
			{'f',	e_dest:E_FLT,		1,	3,	5,	"float"},
			{'f',	e_dest:E_FLTARR,	4,	3,	8,	"floatarr"},
			{'s',	e_dest:E_STRING,	16,	0,	3,	"str"}
		};

		INI_readWhole_NoSections("destination_file.ini", dest_arr, sizeof(dest_arr), struct_v, sizeof(struct_v), "example_file", true);
		INI_saveWhole_NoSections("destination_file.ini", dest_arr, sizeof(dest_arr), struct_v, sizeof(struct_v), "example_file");
**************************************************************/
#define __PAWNINI_INCLUDED__

#if defined INIMODULE_MAX_BUFFER
	#if (INIMODULE_MAX_BUFFER - 0) == 0
		#error You forgot to set value of INIMODULE_MAX_BUFFER. Type something like: #define INIMODULE_MAX_BUFFER (4096).
	#endif
	#if (INIMODULE_MAX_BUFFER < 1)
		#error You are trying to allocate invalid number of unicode general collation table size. Set value of INIMODULE_MAX_BUFFER greather than 0.
	#endif
#else
	#define INIMODULE_MAX_BUFFER		(4096)			// Max size of value.
#endif

#if defined INIMODULE_MAX_VAR_NAME
	#if (INIMODULE_MAX_VAR_NAME - 0) == 0
		#error You forgot to set value of INIMODULE_MAX_VAR_NAME. Type something like: #define INIMODULE_MAX_VAR_NAME (32).
	#endif
	#if (INIMODULE_MAX_VAR_NAME < 1)
		#error You are trying to allocate invalid number of unicode general collation table size. Set value of INIMODULE_MAX_VAR_NAME greather than 0.
	#endif
#else
	#define INIMODULE_MAX_VAR_NAME		(32)			// Max size of value.
#endif

#define INI_TYPE_INT		('i')			// DO NOT CHANGE.
#define INI_TYPE_FLOAT		('f')			// DO NOT CHANGE.
#define INI_TYPE_STRING		('s')			// DO NOT CHANGE.

enum e_ini_struct_info {
	e_type,
	e_offset,
	e_size,
	e_precision,	// signs after dot(for float).
	e_name_len,
	e_name[INIMODULE_MAX_VAR_NAME],
};

//	Check and print ini structure for general errors.
forward INI_isInvalidStruct(const ini_struct[][e_ini_struct_info], const n_ini_parameters = sizeof(ini_struct), const ini_struct_name[]);
//	Validity ini structure in accordance to source/destination array.
forward INI_isInvalidStruct_Dynamic(const ini_struct[][e_ini_struct_info], const n_ini_parameters = sizeof(ini_struct), const destination_size, const ini_struct_name[]);
//
forward INI_readWhole_NoSections(
	const filepath[],						// What file to read.
	array[],								// Array where loaded data must be placed.
	const array_size,						// Number of cells of array.
	const ini_struct[][e_ini_struct_info],	// INI-structure.
	const n_ini_parameters,					// Number of rows in INI-structure.
	const ini_name[],						// INI-structure identify name.
	const bool:only_straight				// If true it is enforces to seek key names in INI-structure in the same order they placed in structure (optimization). Drops everything that out of order.
);
//	Save whole INI file using ini_struct with data from array without sections.
forward INI_saveWhole_NoSections(
	const filepath[],						// Where file to save.
	const array[],							// Array used to export (serialize) data with INI format.
	const array_size,						// Number of cells of array.
	const ini_struct[][e_ini_struct_info],	// INI-structure.
	const n_ini_parameters,					// Number of rows in INI-structure.
	const ini_name[]						// INI-structure identify name.
);


static const comment_symbols[] =			{'#', ';'};
static const invalid_parameter_symbols[] =	{'=', '[', ']'};
static const valid_parameter_type[] =		{INI_TYPE_INT, INI_TYPE_FLOAT, INI_TYPE_STRING};
static const Title_Valid[] =				"INI_isInvalidStruct";
static buffer_string[INIMODULE_MAX_BUFFER + INIMODULE_MAX_VAR_NAME + 16];

//====[Validation]
//	Returns true if parameter_type is valid parameter type (identification).
stock INI_isValidParameterType(const parameter_type) {
	for(new parameter_cell = 0; parameter_cell < sizeof(valid_parameter_type); ++parameter_cell) {
		if( valid_parameter_type[parameter_cell] == parameter_type ) {
			return 1;
		}
	}
	return 0;
}
//	Returns true if parameter name is valid.
stock INI_isValidParameterName(const string[]) {
	if( string[0] == EOS ) {
		return 0;
	}
	new string_cell = 0, string_symbol, invalid_symbol_cell;
	while( (string_symbol = string[string_cell]) ) {
		if( string_symbol < ' ' ) {
			return 0;
		}
		for(invalid_symbol_cell = 0; invalid_symbol_cell < sizeof(invalid_parameter_symbols); ++invalid_symbol_cell) {
			if( string_symbol == invalid_parameter_symbols[invalid_symbol_cell] ) {
				return 0;
			}
		}
		for(invalid_symbol_cell = 0; invalid_symbol_cell < sizeof(comment_symbols); ++invalid_symbol_cell) {
			if( string_symbol == comment_symbols[invalid_symbol_cell] ) {
				return 0;
			}
		}
		string_cell++;
	}
	//	Parameter name have to be trimmed (without spaces/tabs in beginning and end).
	string_cell = 0;
	while( (string_symbol = string[string_cell]) ) {
		if( string_symbol == ' ' || string_symbol == '\t' ) {
			return 0;
		} else {
			break;
		}
		string_cell++;
	}
	string_cell = strlen(string);
	while( string_cell > -1 ) {
		string_symbol = string[string_cell];
		if( string_symbol == ' ' || string_symbol == '\t' ) {
			return 0;
		} else {
			break;
		}
		string_cell--;
	}
	return 1;
}
//	Check and print ini structure for general errors.
stock INI_isInvalidStruct(const ini_struct[][e_ini_struct_info], const n_ini_parameters = sizeof(ini_struct), const ini_struct_name[]) {
	new error_count = 0,
		parameter,
		a_offset_from, a_offset_to,
		b_offset_from, b_offset_to,
		parameter_precision,
		parameter_type,
		parameter_size,
		parameter_name[INIMODULE_MAX_VAR_NAME]
	;
	for(parameter = 0; parameter < n_ini_parameters; ++parameter) {
		strmid(parameter_name, ini_struct[parameter][e_name], 0, INIMODULE_MAX_VAR_NAME, sizeof(parameter_name));
		parameter_type = ini_struct[parameter][e_type];
		parameter_size = ini_struct[parameter][e_size];
		parameter_precision = ini_struct[parameter][e_precision];
		if( !INI_isValidParameterName(parameter_name) ) {
			printf("[%s] error<%d>: found invalid parameter name (\"%s\") (empty or contains invalid symbols) for parameter=%d @ ini_struct_name=\"%s\"!",
				Title_Valid, error_count++, parameter_name, parameter, ini_struct_name
			);
			print(
				"\tInvalid parameter name symbols: any service symbol (code < 32), commentary symbols(#), '=', '[', ']'." \
				"\tParameter name also have to be trimmed: no spaces or tabs in beginning or end."
			);
		}
		if( !INI_isValidParameterType(parameter_type) ) {
			printf("[%s] error<%d>: found invalid parameter_type ('%c') for parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!",
				Title_Valid, error_count++, parameter_type, parameter, parameter_name, ini_struct_name
			);
		}
		if( ini_struct[parameter][e_offset] < 0 ) {
			printf("[%s] error<%d>: found invalid offset=%d (is below 0) for parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!",
				Title_Valid, error_count++, ini_struct[parameter][e_offset], parameter, parameter_name, ini_struct_name
			);
		}
		if( parameter_size < 1 ) {
			printf("[%s] error<%d>: found invalid size=%d (is below 1) for parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!",
				Title_Valid, error_count++, parameter_size, parameter, parameter_name, ini_struct_name
			);
		} else if( parameter_size >= INIMODULE_MAX_BUFFER ) {
			printf("[%s] error<%d>: seems it is too big size=%d (greater than INIMODULE_MAX_BUFFER=%d) for parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!",
				Title_Valid, error_count++, parameter_size, INIMODULE_MAX_BUFFER, parameter, parameter_name, ini_struct_name
			);
		}
		//	Precision test.
		if( parameter_type == INI_TYPE_FLOAT ) {
			if( parameter_precision < 0 || parameter_precision > 8 ) {
				printf("[%s] error<%d>: invalid floating point precision=%d (valid: 0-8) for parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!",
					Title_Valid, error_count++, parameter_precision, parameter, parameter_name, ini_struct_name
				);
			}
		} else {
			if( parameter_precision ) {
				printf("[%s] error<%d>: precision for non float types to be = 0 (current value=%d) for parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!",
					Title_Valid, error_count++, parameter_precision, parameter, parameter_name, ini_struct_name
				);
			}
		}
		//	Name length size test.
		if( strlen(parameter_name) != ini_struct[parameter][e_name_len] ) {
			printf("[%s] error<%d>: invalid e_name_len=%d instead of %d for parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!",
				Title_Valid, error_count++, ini_struct[parameter][e_name_len], strlen(parameter_name), parameter, ini_struct[parameter][e_name], ini_struct_name
			);
		}
		//	Find collisions.
		a_offset_from =	(ini_struct[parameter][e_offset]);
		a_offset_to =	(parameter_size + a_offset_from);
		for(new parameter_another = parameter + 1; parameter_another < n_ini_parameters; ++parameter_another) {
			b_offset_from =	(ini_struct[parameter_another][e_offset]);
			b_offset_to =	(ini_struct[parameter_another][e_size] + b_offset_from);
			if(
				_intrange_testIntersect(a_offset_from, a_offset_to, b_offset_from, b_offset_to) ||
				_intrange_testIntersect(b_offset_from, b_offset_to, a_offset_from, a_offset_to) ||
				_intrange_testContain(a_offset_from, a_offset_to, b_offset_from, b_offset_to) ||
				_intrange_testContain(b_offset_from, b_offset_to, a_offset_from, a_offset_to)
			) {
				printf(
					"[%s] error<%d>: found collision between parameter=%d (\"%s\") & parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!\n" \
					"\tDestination ranges are intersecting!",
					Title_Valid, error_count++,
					parameter, parameter_name,
					parameter_another, ini_struct[parameter_another][e_name],
					ini_struct_name
				);
			}
			if( !strcmp(parameter_name, ini_struct[parameter_another][e_name], false) ) {
				printf(
					"[%s] error<%d>: found parameters with same name: parameter=%d & parameter=%d (\"%s\") @ ini_struct_name=\"%s\"!",
					Title_Valid, error_count++,
					parameter, parameter_another, parameter_name
				);
			}
		}
	}
	#if !defined INIMODULE_ERRORSTRUCT_NONFATAL
	if( error_count ) {
		printf(
			"[%s]: you have %d unfixed errors in ini structure definition (\"%s\") [called from OR]\n" \
			"\tStop asking stupid questions on forums - I gave you all YOUR problems you have to fix in YOUR code.\n" \
			"\tUse translator if you don't understand!",
			Title_Valid, error_count, ini_struct_name
		);
		exit(1);
	}
	#endif
	return error_count;
}
//	Validity ini structure in accordance to source/destination array.
stock INI_isInvalidStruct_Dynamic(const ini_struct[][e_ini_struct_info], const n_ini_parameters = sizeof(ini_struct), const destination_size, const ini_struct_name[]) {
	new parameter, from, to, error_count = 0;
	for(parameter = 0; parameter < n_ini_parameters; ++parameter) {
		from = ini_struct[parameter][e_offset];
		to = ini_struct[parameter][e_size] + from;
		if( to > destination_size ) {
			printf(
				"[INI_isInvalidStruct_Dynamic]: error<%d> parameter=%d (\"%s\") is out of destination bounds offset_from=%d, offset_to=%d, destination_size=%d @ INI structure (\"%s\")!",
				error_count++,
				parameter, ini_struct[parameter][e_name], from, to, destination_size,
				ini_struct_name
			);
		}
	}
	#if !defined INIMODULE_ERRORSTRUCT_NONFATAL
	if( error_count ) {
		printf("[INI_isInvalidStruct_Dynamic]: you have %d some problems @ ini structure (\"%s\") with destination_size. Fix them before next run.", error_count, ini_struct_name);
		exit(1);
	}
	#endif
	return error_count;
}

static stock _intrange_testIntersect(const afrom, const ato, const bfrom, const bto) {
	return (
		((afrom >= bfrom) && (afrom < bto)) ||
		((bfrom >= afrom) && (bfrom < ato))
	);
}
static stock _intrange_testContain(const afrom, const ato, const bfrom, const bto) {
	return (
		(afrom >= bfrom) && (ato <= bto)
	);
}
//===============
//	Load whole INI file using ini_struct with data from array without sections.
stock INI_readWhole_NoSections(
	const filepath[],						// What file to read.
	array[],								// Array where loaded data must be placed.
	const array_size,						// Number of cells of array.
	const ini_struct[][e_ini_struct_info],	// INI-structure.
	const n_ini_parameters,					// Number of rows in INI-structure.
	const ini_name[],						// INI-structure identify name.
	const bool:only_straight				// If true it is enforces to seek key names in INI-structure in the same order they placed in structure (optimization). Drops everything that out of order.
) {
	#if !defined INIMODULE_IGNORE_STRUCT_CHECK
	if(
		INI_isInvalidStruct(ini_struct, n_ini_parameters, ini_name) ||
		INI_isInvalidStruct_Dynamic(ini_struct, n_ini_parameters, array_size, ini_name)
	) {
		printf("[INI_readWhole_NoSections] cant read data from (\"%s\") due to errors in ini structure!", ini_name);
		return 0;
	}
	#else
	#pragma unused ini_name
	#pragma unused array_size
	#endif
	new File:f = fopen(filepath, io_read);
	if( f ) {
		new buffer_string_len,
			variable_name[INIMODULE_MAX_VAR_NAME],
			variable_name_size,
			variable_index,
			parameter_pos,
			parameter_type,
			parameter_size,
			parameter_offset,
			parameter,
			last_parameter = 0,
			line_index = 0,
			bool:key_is_found
		;
		while( (buffer_string_len = fread(f, buffer_string)) ) {
			line_index++;
			buffer_string_len = string_clear_newline_tail(buffer_string, buffer_string_len);
			buffer_string_len = string_clear_from_commentary(buffer_string, buffer_string_len, comment_symbols, sizeof(comment_symbols));
			if( (buffer_string_len > 2) && (buffer_string[0] != '=' && buffer_string[0] != '[') ) {	// At least 3 symbols and not anonymous. Example: "a=1".
				string_extract_ini_components(buffer_string, buffer_string_len, parameter_pos, variable_index, variable_name_size);
				if( variable_name_size >= INIMODULE_MAX_VAR_NAME ) {
					#if !defined INIMODULE_SILENT_RUNTIME_WARNS
					printf("[INI_readWhole_NoSections] %s@%s:%d found too long key name. Ignored <%s>!", filepath, ini_name, line_index, buffer_string);
					#endif
					continue;
				}
				strmid(variable_name, buffer_string, 0, variable_name_size, sizeof(variable_name));
				key_is_found = false;
				for(parameter = last_parameter; parameter < n_ini_parameters; ++parameter) {
					if( (ini_struct[parameter][e_name_len] == variable_name_size) && !strcmp(ini_struct[parameter][e_name], buffer_string, false, variable_name_size) ) {
						key_is_found = true;
						parameter_type =	ini_struct[parameter][e_type];
						parameter_size =	ini_struct[parameter][e_size];
						parameter_offset =	ini_struct[parameter][e_offset];
						if( parameter_type == INI_TYPE_INT ) {
							if( variable_index < parameter_size ) {
								array[parameter_offset + variable_index] = strval(buffer_string[parameter_pos]);
							}
							#if !defined INIMODULE_SILENT_RUNTIME_WARNS
							else {
								printf("[INI_readWhole_NoSections] %s@%s:%d trying to access invalid index(%d) while size=%d!", filepath, ini_name, line_index, variable_index, parameter_size);
							}
							#endif
						} else if( parameter_type == INI_TYPE_FLOAT ) {
							if( variable_index < parameter_size ) {
								array[parameter_offset + variable_index] = _:floatstr(buffer_string[parameter_pos]);
							}
							#if !defined INIMODULE_SILENT_RUNTIME_WARNS
							else {
								printf("[INI_readWhole_NoSections] %s@%s:%d trying to access invalid index(%d) while size=%d!", filepath, ini_name, line_index, variable_index, parameter_size);
							}
							#endif
						} else if( parameter_type == INI_TYPE_STRING ) {
							strmid(array[parameter_offset], buffer_string, parameter_pos, buffer_string_len, parameter_size);
						}
						if( only_straight ) {
							last_parameter = parameter;
						}
						break;
					}
				}
				#if !defined INIMODULE_SILENT_RUNTIME_WARNS
				if( !key_is_found ) {
					printf("[INI_readWhole_NoSections] %s@%s:%d found unknown key name (%s)!", filepath, ini_name, line_index, variable_name);
				}
				#endif
			}
		}
		fclose(f);
		return 1;
	}
	return 0;
}

//	Save whole INI file using ini_struct with data from array without sections.
stock INI_saveWhole_NoSections(
	const filepath[],						// Where file to save.
	const array[],							// Array used to export (serialize) data with INI format.
	const array_size,						// Number of cells of array.
	const ini_struct[][e_ini_struct_info],	// INI-structure.
	const n_ini_parameters,					// Number of rows in INI-structure.
	const ini_name[]						// INI-structure identify name.
) {
	#if !defined INIMODULE_IGNORE_STRUCT_CHECK
	if(
		INI_isInvalidStruct(ini_struct, n_ini_parameters, ini_name) ||
		INI_isInvalidStruct_Dynamic(ini_struct, n_ini_parameters, array_size, ini_name)
	) {
		printf("[INI_saveWhole_NoSections] cant save data to (\"%s\") due to errors in ini structure!", ini_name);
		return 0;
	}
	#endif
	new File:f = fopen(filepath, io_write);
	if( f ) {
		new parameter, parameter_type, parameter_size, parameter_offset, parameter_subindex, parameter_precision;
		for(parameter = 0; parameter < n_ini_parameters; ++parameter) {
			parameter_type =		ini_struct[parameter][e_type];
			parameter_size =		ini_struct[parameter][e_size];
			parameter_offset =		ini_struct[parameter][e_offset];
			parameter_precision =	ini_struct[parameter][e_precision];
			if( parameter_type == INI_TYPE_INT ) {
				if( parameter_size > 1 ) {
					for(parameter_subindex = 0; parameter_subindex < parameter_size; ++parameter_subindex) {
						format(buffer_string, sizeof(buffer_string), "%s[%d]=%d\n", ini_struct[parameter][e_name], parameter_subindex, array[parameter_offset + parameter_subindex]);
						fwrite(f, buffer_string);
					}
				} else {
					format(buffer_string, sizeof(buffer_string), "%s=%d\n", ini_struct[parameter][e_name], array[parameter_offset]);
					fwrite(f, buffer_string);
				}
			} else if( parameter_type == INI_TYPE_FLOAT ) {
				if( parameter_size > 1 ) {
					for(parameter_subindex = 0; parameter_subindex < parameter_size; ++parameter_subindex) {
						format(buffer_string, sizeof(buffer_string), "%s[%d]=%0.*f\n", ini_struct[parameter][e_name], parameter_subindex, parameter_precision, array[parameter_offset + parameter_subindex]);
						fwrite(f, buffer_string);
					}
				} else {
					format(buffer_string, sizeof(buffer_string), "%s=%0.*f\n", ini_struct[parameter][e_name], parameter_precision, array[parameter_offset]);
					fwrite(f, buffer_string);
				}
			} else if( parameter_type == INI_TYPE_STRING ) {
				format(buffer_string, sizeof(buffer_string), "%s=", ini_struct[parameter][e_name]);
				fwrite(f, buffer_string);
				fwrite(f, array[parameter_offset]);	// separate write to support UTF8 strings.
				fwrite(f, "\n");
			}
		}
		fclose(f);
		return 1;
	}
	return 0;
}

static stock string_clear_newline_tail(string[], string_len) {
	new string_cell = string_len;
	while(
		(-1 < --string_cell) &&
		(string[string_cell] == '\n' || string[string_cell] == '\r')
	) {
		string[string_cell] = EOS;
		string_len = string_cell;
	}
	return string_len;
}
static stock string_clear_from_commentary(string[], const string_len, const commentary_symbols[], const n_symbols) {
	new string_cell,
		string_symbol,
		commentary_symbol_cell
	;
	for(string_cell = 0; string_cell < string_len; ++string_cell) {
		string_symbol = string[string_cell];
		for(commentary_symbol_cell = 0; commentary_symbol_cell < n_symbols; ++commentary_symbol_cell) {
			if( (string_symbol == commentary_symbols[commentary_symbol_cell]) &&
				(string_cell > 0 && string[string_cell - 1] != '\\' )
			) {
				string[string_cell] = EOS;
				return string_cell;
			}
		}
	}
	return string_len;
}
static stock string_extract_ini_components(const string[], const string_len, &parameter_pos, &variable_index, &variable_name_size) {
	new string_cell,
		string_symbol,
		variable_index_start = -1
	;
	variable_index = 0;
	variable_name_size = 0;
	for(string_cell = 1; string_cell < string_len; ++string_cell) {
		string_symbol = string[string_cell];
		if( string_symbol == '=' ) {
			if( !variable_name_size ) {
				variable_name_size = string_cell;
			}
			parameter_pos = string_cell + 1;
			break;
		} else if( string_symbol == '[' ) {
			if( !variable_name_size ) {
				variable_name_size = string_cell;
			}
			variable_index_start = string_cell + 1;
		} else if( string_symbol == ']' ) {
			if( variable_index_start > 0 ) {
				new string_variable_index[16];
				strmid(string_variable_index, string, variable_index_start, string_cell, sizeof(string_variable_index));
				variable_index = strval(string_variable_index);
				variable_index_start = -1;
			}
		}
	}
}
