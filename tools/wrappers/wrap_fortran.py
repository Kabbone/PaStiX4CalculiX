#!/usr/bin/env python
"""
Wrapper Fortran 90
==================

 @file wrappers/wrap_fortran.py

 PaStiX generator for the Fortran 90 wrapper

 @copyright 2017-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
                      Univ. Bordeaux. All rights reserved.

 @version 6.4.0
 @author Mathieu Faverge
 @author Tony Delarue
 @author Selmane Lebdaoui
 @date 2024-07-05

"""
import os
import re
import argparse
import time
from . import *
from .pastix_fortran import *

def function_register_derived_type( function, subset, ctype ):
    if not 'derived_types' in function.keys():
        function['derived_types'] = { 'cside' : {}, 'fside' : {} }

    derived_types = function['derived_types'][subset]
    use  = ctype['use']
    only = ctype['only']

    if subset == 'cside' and only == 'MPI_Comm':
        use  = 'iso_c_binding'
        only = 'c_int'

    if not use in derived_types.keys():
        derived_types[use] = set()

    derived_types[use].add(only)

def function_prepare_arg_for_bindings( function, arg, return_value ):
    """
    Update the arg structure to discover the different associated fields.

     - c_type: the associated fortran datatype
     - c_pointer: specifiy in the interface if this is a value or a pointer

    Also update the size dictionnary to know the maximum length of
    each field for nice formatting
    """

    is_pointer = ( arg['pointer'] > 0 )
    if is_pointer:
        arg['c_type'] = "type(c_ptr)"
        ctmp = { 'use' : "iso_c_binding", 'only' : "c_ptr" }
        function_register_derived_type( function, 'cside', ctmp )
    else:
        dtype = arg['type']
        if dtype == "MPI_Comm":
            dtype = "int"

        type_dict = types_dict[dtype]
        arg['c_type'] = type_dict['ftype']
        function_register_derived_type( function, 'cside', type_dict )

    arg['c_pointer'] = ""
    connect = 0
    if (not return_value) and (arg['pointer'] < 2):
        connect = 2 # ", "
        arg['c_pointer'] = "value"

    # Update the maximum length
    sizes = function['sizes']
    sizes['ctype']   = max(sizes['ctype'],   len( arg['c_type'])    )
    sizes['pointer'] = max(sizes['pointer'], len( arg['c_pointer']) )
    sizes['maxlenc'] = max(sizes['maxlenc'], len( arg['c_type'] + arg['c_pointer'] )+connect )

def function_prepare_arg_for_interface( function, arg, return_value ):
    """
    Update the arg structure to discover the different associated fields.

     - f_type: the assiacted fortran datatype
     - f_pointer: specifiy in the interface if this is a value or a pointer
     - f_intent: in,out,inout specification of the argument
     - f_target: specify if a pointer is a target or a pointer
     - f_array: specify the dimensions when arrays are given

    Also update the size dictionnary to know the maximum length of
    each field for nice formatting
    """

    is_pointer = ( arg['pointer'] > 0 )
    arg['has_aux'] = ( arg['pointer'] > 1 )
    arg['is_optional'] = False

    # Register c_null_ptr if needed
    if arg['type'] == "FILE":
        ftmp = { 'use' : "iso_c_binding", 'only' : "c_null_ptr" }
        function_register_derived_type( function, 'fside', ftmp )

        # We don't do anything with FILE on the fortran side
        return

    if return_value and (arg['type'] == "int"):
        arg['has_aux']     = True
        arg['is_optional'] = True

    if "opt_" in arg['name']:
        arg['has_aux']     = True
        arg['is_optional'] = True

    # Variables pour cas optionnels
    if arg['is_optional']:
        if is_pointer:
            ctmp = { 'use' : "iso_c_binding", 'only' : "c_ptr" }
            function_register_derived_type( function, 'fside', ctmp )
            ctmp = { 'use' : "iso_c_binding", 'only' : "c_null_ptr" }
            function_register_derived_type( function, 'fside', ctmp )
        if arg['type'] == "MPI_Comm":
            ctmp = { 'use' : "spmf_enums", 'only' : "MPI_COMM_WORLD" }
            function_register_derived_type( function, 'fside', ctmp )

    # Register functions used for pointers
    if is_pointer:
        if arg['type'] != "void":
            ftmp = { 'use' : "iso_c_binding", 'only' : "c_loc" }
            function_register_derived_type( function, 'fside', ftmp )
            arg['f2c'] = "c_loc"
        else:
            vname = re.sub(r"^opt_", "", arg['name'], flags=re.IGNORECASE)
            if vname in arrays_names_2D:
                arg['f2c'] = "pastixGetCptrFrom2dArray"
            elif vname in arrays_names_1D:
                arg['f2c'] = "pastixGetCptrFrom1dArray"
            else:
                arg['f2c'] = "pastixGetCptrFromValue"

            ftmp = { 'use' : "pastixf_bindings", 'only' : arg['f2c'] }
            function_register_derived_type( function, 'fside', ftmp )
            arg['has_aux'] = True

        if arg['pointer'] > 1:
            ftmp = { 'use' : "iso_c_binding", 'only' : "c_ptr" }
            function_register_derived_type( function, 'fside', ftmp )

            ftmp = { 'use' : "iso_c_binding", 'only' : "c_f_pointer" }
            function_register_derived_type( function, 'fside', ftmp )

    # Type string
    arg['f_type'] = types_dict[arg['type']]['ftype']
    if (not return_value) or (function['symbol'] == 'function'):
        function_register_derived_type( function, 'fside', types_dict[arg['type']] )
    arg['f_type'] += ", "

    # Intent string
    arg['f_intent'] = "intent(in)"
    if return_value:
        arg['f_intent'] = "intent(out)"
    elif is_pointer and (not arg['const']):
        arg['f_intent'] = "intent(inout)"

    # Target string
    count=0
    arg['f_target'] = ""
    if is_pointer:
        count += 1
        arg['f_intent'] += ", "
        if return_value:
           arg['f_target'] = "pointer"
        else:
            if arg['pointer'] == 1:
                arg['f_target'] = "target"
                # if arg['type'] == "void":
                #     arg['f_type']   = arg['f_type'][:-2]
                #     arg['f_target'] = ""
                #     arg['f_intent'] = ""
            else:
                arg['f_target'] = "pointer"

    # Optional string
    arg['f_option'] = ""
    if arg['is_optional']:
        count += 1
        if arg['f_target'] != "":
            arg['f_target'] += ", "
        else:
            arg['f_intent'] += ", "
        arg['f_option'] = "optional"

    # Array string
    arg['f_array'] = ""
    if is_pointer:
        vname = re.sub(r"^opt_", "", arg['name'], flags=re.IGNORECASE)
        if vname in arrays_names_2D:
            arg['f_array'] = "(:,:)"
        elif vname in arrays_names_1D:
            arg['f_array'] = "(:)"

    sizes = function['sizes']
    sizes['type']   = max(sizes['type'],   len( arg['f_type'])   )
    sizes['intent'] = max(sizes['intent'], len( arg['f_intent']) )
    sizes['option'] = max(sizes['option'], len( arg['f_option']) )
    sizes['target'] = max(sizes['target'], len( arg['f_target']) )

    function['variable_fmt'] = max( function['variable_fmt'], count )
    return 0

def print_function_use( s, function, subset, interface=True, maxlen=0 ):
    def module_len( derived_types, x ):
        tmp = list()
        for elem in sorted( derived_types[x] ):
            if interface and (elem in ( "c_loc", "c_null_ptr", "c_f_pointer" )):
                continue
            tmp.append( elem )

        if len(tmp) > 0:
            return len(x)
        else:
            return 0

    string = ""
    if 'derived_types' in function.keys():

        derived_types = function['derived_types'][subset]

        maxlen2 = max( map(lambda x: module_len( derived_types, x ), derived_types.keys()) )
        maxlen = max( maxlen, maxlen2 )

        for module in sorted( derived_types.keys() ):
            line = s*" " + "use :: " + module + "," + (maxlen - len(module))*" " + " only : "

            j = 0
            for elem in sorted( derived_types[module] ):
                if interface and (elem in ( "c_loc", "c_null_ptr", "c_f_pointer" )):
                    continue
                if j > 0:
                    line += ", "
                line += elem
                j += 1
            line += "\n"

            if j > 0:
                string += line

    string += s*" " + "implicit none\n"
    return string

# loop over the arguments to compose the first line
def print_function_parameters( s, function, initial_len, drop_file=True, retval=False ):
    s += iindent
    j = 1
    string = ""
    for arg in function['args']:
        if drop_file and arg['type'] == "FILE":
            continue

        if (j != 1):
            string += ", "
            initial_len += 2

        l = len( arg['name'] )
        if (initial_len + l) > 77:
            string += "&\n" + s*" "
            initial_len = s

        string += arg['name']
        initial_len += l
        j += 1

    if retval and function['is_function']:
        if (j != 1):
            string += ", "
            initial_len += 2

        name = return_variables_dict[ function['rettype']['type'] ]
        l = len( name )
        if (initial_len + l) > 77:
            string += "&\n" + s*" "
            initial_len = s

        string += name
        initial_len += l

    string += ")\n"
    s -= iindent
    return string

# loop over the arguments to compose the first line
def print_function_parameters_c( shift, function ):
    maxlen = 0
    for arg in function['args']:
        l = 0
        if arg['const']:
            l += 6

        argtype = arg['type']
        if argtype == "MPI_Comm":
            argtype = "int"

        l += len( argtype )
        l += arg['pointer']

        maxlen = max( maxlen, l+1 )

    j = 1
    s = 0
    string = ""
    for arg in function['args']:
        if (j != 1):
            string += ",\n"

        if arg['const']:
            argconst = "const "
        else:
            argconst = ""

        argtype = arg['type']
        if argtype == "MPI_Comm":
            argtype = "int"

        argpointer = arg['pointer']*"*"

        fmt = s*" " + "%s%s%" + str( maxlen - len(argconst + argtype)) + "s%s"
        string += format( fmt % (argconst, argtype, argpointer, arg['name']) )
        j += 1
        s = shift
    if len(function['args']) == 0:
        string += "void"

    string += " )\n"
    return string

# loop over the arguments to compose the first line
def print_function_call( s, function ):
    # is it a function or a subroutine
    str_call = "call "
    str_call_end = ")"
    if function['is_function']:
        rettype = function['rettype']
        if rettype['type'] == "int":
            return_var = "info"
        else:
            return_var = return_variables_dict[rettype['type']]

        if rettype['has_aux']:
            return_var = "x_" + return_var

        if rettype['pointer'] > 0:
            str_call = "call c_f_pointer("
            str_call_end = "), " + return_var + ")"
        else:
            str_call = return_var + " = "

    string = s*" " + str_call + function['name'] + "_f2c" + "("
    initial_len = len( string )
    s += itab + iindent
    j = 1
    for arg in function['args']:
        if (j != 1):
            string += ", "
            initial_len += 2

        if arg['has_aux']:
            name = "x_" + arg['name']
        else:
            name = arg['name']

        if arg['type'] == "FILE":
            # FILE are not correctly handled
            str_arg = "c_null_ptr"
        elif arg['type'] == "MPI_Comm":
            # FILE are not correctly handled
            str_arg = name + "%MPI_VAL"
        elif arg['has_aux']:
            str_arg = name
        elif arg['pointer'] > 0:
            str_arg = "c_loc(" + arg['name'] + ")"
        else:
            str_arg = arg['name']

        l = len( str_arg )
        if (initial_len + l) > 77:
            string += "&\n" + s*" "
            initial_len = s

        string += str_arg
        initial_len += l
        j += 1

    s -= (itab + iindent)
    string += str_call_end + "\n"
    return string

# loop over the arguments to compose the first line
def print_function_call_c( s, function ):
    # is it a function or a subroutine
    str_call = function['name'] + "("
    str_call_end = " );"
    if function['is_function']:
        str_call = "return " + str_call

    string = s*" " + str_call
    s = len( string )
    j = 1
    initial_len = s
    for arg in function['args']:
        if (j != 1):
            string += ","
            initial_len += 1

        if arg['type'] == "MPI_Comm":
            str_arg = "_pastix_comm_f2c( " + arg['name'] + " )"
        else:
            str_arg = arg['name']

        l = 1 + len( str_arg )
        if (initial_len + l) > 80:
            string += "\n" + s*" "
            initial_len = s

        string += " " + str_arg
        initial_len += l
        j += 1

    s -= (itab + iindent)
    string += str_call_end + "\n"
    return string

def print_binding_variable_list( s, function ):
    def binding_print_param( param, maxlen, s ):
        strtype = param['c_type']
        if len(param['c_pointer']) > 0:
            strtype += ", "
        fmt = s*" " + "%s%" + str( maxlen - len(strtype) ) + "s :: %s\n"
        return format( fmt % (strtype, param['c_pointer'], param['name']) )

    string  = ""
    maxlenc = function['sizes']['maxlenc']

    # Print the return value
    if function['symbol'] == "function":
        string += binding_print_param( function['rettype'], maxlenc, s )

    # Print all the parameters
    for arg in function['args']:
        string += binding_print_param( arg, maxlenc, s )

    return string

def print_function_variable_list( s, function, interface ):
    maxlen = function['sizes']
    if function['variable_fmt'] == 2:
        subfmt = "%-" + str(maxlen['target']) + "s%-" + str(maxlen['option']) + "s"
    else:
        maxsub = max( maxlen['target'], maxlen['option'] )
        subfmt = "%-" + str(maxsub) + "s"
    fmt = s*" " + "%-" + str(maxlen['type']) + "s%-" + str(maxlen['intent']) + "s%s :: %s%s\n"

    string = ""
    for arg in function['args']:
        if arg['name'] == '':
            # void function
            return string
        if arg['type'] == "FILE":
            continue
        ftype = arg['f_type']
        if arg['type'] == "void":
            ftype = "class(*),"

        if function['variable_fmt'] == 2:
            tmp = format( subfmt % ( arg['f_target'], arg['f_option'] ) )
        else:
            tmp = format( subfmt % ( arg['f_target'] + arg['f_option'] ) )
        string += format( fmt % ( ftype, arg['f_intent'], tmp, arg['name'], arg['f_array'] ) )

    if not interface and function['is_function']:
        arg  = function['rettype']
        name = return_variables_dict[arg['type']]

        if function['variable_fmt'] == 2:
            tmp = format( subfmt % ( arg['f_target'], arg['f_option'] ) )
        else:
            tmp = format( subfmt % ( arg['f_target'] + arg['f_option'] ) )
        string += format( fmt % ( arg['f_type'], arg['f_intent'], tmp, name, arg['f_array'] ) )

    return string

def print_auxiliary_variables( s, function ):
    def auxtype_len( larg ):
        if not larg['has_aux']:
            return 0
        if larg['type'] == "MPI_Comm":
            return len(larg['f_type'])-2
        else:
            return len(larg['c_type'])

    def auxname_len( larg ):
        if not larg['has_aux']:
            return 0
        return len(larg['name'])

    maxlenn = 0
    maxlent = 0

    if len( function['args'] ) > 0:
        maxlenn = max( map(lambda x: auxname_len( x ), function['args']) )
        maxlent = max( map(lambda x: auxtype_len( x ), function['args']) )

    if function['is_function'] and function['rettype']['has_aux']:
        maxlent = max( maxlent, len(function['rettype']['f_type'])-2 )

    fmt = s*" " + "%-" + str(maxlent) + "s :: %s"

    string = ""
    for arg in function['args']:
        if not arg['has_aux']:
            continue

        ftype = arg['c_type']
        if arg['type'] == "MPI_Comm":
            ftype = arg['f_type'][:-2]
        string += format( fmt % ( ftype, "x_" + arg['name'] ) )

        if arg['is_optional']:
            string += (maxlenn - len(arg['name']))*" " + " = "
            if arg['pointer'] > 0:
                string += "c_null_ptr"
            elif arg['type'] in ( "int", "spm_int_t" ):
                string += "1"
            elif arg['type'] in "MPI_Comm":
                string += "MPI_COMM_WORLD"

        string += "\n"

    arg = function['rettype']
    if function['is_function'] and arg['has_aux']:
        ftype = arg['f_type'][:-2]
        if arg['type'] == "void":
            ftype = "type(c_ptr)"
        name = return_variables_dict[ arg['type'] ]
        string += format( fmt % ( ftype, "x_" + name ) )
        string += "\n"

    return string

def print_auxiliary_init( s, function ):
    def auxinit_len( larg ):
        if (not larg['has_aux']) or (['f_intent'] == "intent(out)"):
            return 0
        return len(larg['name'])+2

    if len( function['args'] ) == 0:
        return ""
    maxlen = max( map(lambda x:  auxinit_len( x ), function['args']) )
    fmt = "%-" + str(maxlen) + "s = "

    string = ""
    for arg in function['args']:
        if not arg['has_aux']:
            continue

        if ['f_intent'] == "intent(out)":
            continue

        if arg['is_optional']:
            string += s*" " + "if ( present(" + arg['name'] + ") )" + (maxlen-1-len(arg['name']))*" "
        else:
            string += s*" "

        string += format( fmt % ( "x_" + arg['name'] ) )

        if arg['pointer'] > 0:
            string += arg['f2c'] + "(" + arg['name'] +")\n"
        else:
            string += arg['name']+"\n"

    return string

def print_auxiliary_fini( s, function ):
    string = ""
    for arg in function['args']:
        if not arg['has_aux']:
            continue

        if arg['pointer'] > 1:
            string += s*" " + "call c_f_pointer(" + "x_" + arg['name'] + ", " + arg['name'] +")\n"

    arg = function['rettype']
    if function['is_function'] and arg['is_optional']:
        name = return_variables_dict[ arg['type'] ]

        string += s*" " + "if ( present(" + name + ") ) " + name + " = x_" + name
        string += "\n"

    return string

def iso_c_interface_type(arg, return_value, list, instruct=False):
    """Generate a declaration for a variable in the interface."""

    if (arg[1] == "*" or arg[1] == "**"):
        is_pointer = True
    else:
        is_pointer = False

    if (is_pointer):
        f_type = "type(c_ptr)"
    else:
        f_type = types_dict[arg[0]]['ftype']
        if instruct and f_type == "type(MPI_Comm)":
            f_type = "integer(kind=PASTIX_MPI_COMM_KIND)"

    if (not return_value and arg[1] != "**"):
        f_type += ", "
        f_pointer = "value"
    else:
        f_pointer = ""

    f_name = arg[2]

    list.append( [ f_type, f_pointer, f_name ] );
    return len(f_type + f_pointer)

class wrap_fortran:

    @staticmethod
    def write_header( f, module=True ):
        filename = os.path.basename( f['filename'] )
        header = '''!>
!> @file '''+ filename +'''
!>
!> ''' + f['description'] + '''
!>
!> @copyright 2017-''' + time.strftime( "%Y" ) + ''' Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
!>                      Univ. Bordeaux. All rights reserved.
!>
!> @version 6.4.0
!> @author Mathieu Faverge
!> @author Tony Delarue
!> @author Selmane Lebdaoui
!> @date ''' + time.strftime( "%Y-%m-%d" ) + '''
!>
!> This file has been automatically generated with gen_wrappers.py
!>
!> @ingroup wrap_fortran
!>'''
        if module:
            modname = re.sub(r".f90", "", filename, flags=re.IGNORECASE)
            header += "\nmodule " + modname

        if f['header'] != "":
            header += f['header']
        else:
            header += "\n"

        return header

    @staticmethod
    def write_footer( f, module=True ):
        footer = f['footer']

        if module:
            filename = os.path.basename( f['filename'] )
            modname = re.sub(r".f90", "", filename, flags=re.IGNORECASE)
            footer += "end module " + modname + "\n"
        return footer

    @staticmethod
    def write_header_c( f ):
        filename = os.path.basename( f['filename'] )
        header = '''/**
 * @file '''+ filename +'''
 *
 * ''' + f['description'] + '''
 *
 * @copyright 2017-''' + time.strftime( "%Y" ) + ''' Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Mathieu Faverge
 * @author Tony Delarue
 * @date ''' + time.strftime( "%Y-%m-%d" ) + '''
 *
 * This file has been automatically generated with gen_wrappers.py
 *
 * @ingroup wrap_fortran
 *
 */'''
        if f['header'] != "":
            header += f['header']
        else:
            header += "\n"

        return header

    @staticmethod
    def write_footer_c( f ):
        footer = f['footer']
        return footer

    @staticmethod
    def write_enum( f, enum ):
        """Generate an interface for an enum.
           Translate it into constants."""

        ename  = enum[0]
        params = enum[1]

        # initialize a string with the fortran interface
        str_enum  = tab + "! enum " + ename + "\n"
        str_enum += tab + "enum, bind(C)\n"

        # loop over the arguments of the enum to get max param length
        length=0
        for param in params:
            length= max( length, len(param[0]) )
        fmt="%-"+ str(length) + "s"

        # Increment for index array enums
        inc = 0
        if ename[1:5] == "parm":
            inc=1

        # loop over the arguments of the enum
        for param in params:
            name  = param[0]
            if isinstance(param[1],int):
                if name[1:10] == "PARM_SIZE":
                    value = str(param[1])
                else:
                    value = str(param[1] + inc)
            else:
                value = str(param[1])
            str_enum += tab + "   enumerator :: " + format(fmt % name) + " = " + value + "\n"

        str_enum += tab + "end enum\n\n"
        return str_enum

    @staticmethod
    def write_struct(struct):
        """Generate an interface for a struct.
           Translate it into a derived type."""

        # initialize a string with the fortran interface
        str_struct = ""

        s = itab
        name = struct[0][2]
        str_struct += s*" " + "type, bind(c) :: " + name + "\n"

        # loop over the arguments of the struct to get the length
        s += iindent
        slist = []
        length= 0
        for j in range(1,len(struct)):
            length = max( length, iso_c_interface_type(struct[j], True, slist, instruct=True) )
        fmt = s*" " + "%-"+ str(length) + "s :: %s\n"

        # loop over the arguments of the struct
        for j in range(0,len(slist)):
            str_struct += format( fmt % (slist[j][0], slist[j][2]) )

        s -= iindent
        str_struct += s*" " + "end type " + name + "\n"

        return str_struct

    @staticmethod
    def prepare_function( function ):
        """Generate an interface for a function."""

        fname   = function['name']
        fname_f = fname + "_f08"
        rettype = function['rettype']

        sizes = { 'ctype' : 0, 'maxlenc' : 0,
                  'type' : 0, 'pointer' : 0, 'intent' : 0, 'option' : 0, 'target' : 0 }
        function['sizes']  = sizes
        function['symbol'] = "subroutine"
        function['variable_fmt'] = 0

        # Is it a function or a subroutine ?
        if function['is_function']:
            function['symbol'] = "function"
            rettype['name']    = fname_f

            # Register c_f_pointer if needed
            if rettype['pointer'] > 0:
                ftmp = { 'use' : "iso_c_binding", 'only' : "c_f_pointer" }
                function_register_derived_type( function, 'fside', ftmp )

        function_prepare_arg_for_bindings( function, rettype, True )
        function_prepare_arg_for_interface( function, rettype, True )

        #
        # List the derived data types
        #
        for arg in function['args']:
            function_prepare_arg_for_bindings( function, arg, False )
            function_prepare_arg_for_interface( function, arg, False )

        function['sizes']  = sizes

    @staticmethod
    def write_interface( function ):
        """Generate an interface for a function."""

        fname   = function['name']
        fname_f = fname + "_f08"
        rettype = function['rettype']
        symbol  = "subroutine" #function['symbol']

        # Initialize a string with the fortran interface
        s = itab
        str_interface = s*" " + "interface " + fname + "\n"
        s += iindent

        str_interface += s*" " + symbol + " " + fname_f + "("
        initial_len = s + len( symbol + " " + fname_f + "(" )
        s += itab

        # loop over the arguments to compose the first line
        str_interface += print_function_parameters( s, function, initial_len, True, True )
        str_interface += print_function_use( s, function, 'fside', True )
        str_interface += print_function_variable_list( s, function, False )

        s -= itab
        str_interface += s*" " + "end " + symbol + " " + fname_f + "\n"

        s -= iindent
        str_interface += s*" " + "end interface " + fname + "\n\n"

        return str_interface

    @staticmethod
    def write_binding( function ):
        """Generate an interface for a function."""

        fname   = function['name']
        fname_f = fname + "_f2c"
        rettype = function['rettype']
        rettype['name'] = fname_f
        symbol  = function['symbol']

        # Initialize a string with the fortran interface
        s = itab + iindent
        str_declaration = symbol + " " + fname_f
        str_binding = "\n" + s*" " + str_declaration + "("
        initial_len = s + len( str_declaration + "(" )
        s += itab

        str_binding += print_function_parameters( s, function, initial_len, False, False )

        # Add the C binding name
        s += iindent
        str_binding = str_binding[:-1] + " &\n"
        str_binding += s*" " + "bind(c, name='" + fname_f +"')\n"
        s -= iindent

        str_binding += print_function_use( s, function, 'cside', True )
        str_binding += print_binding_variable_list( s, function )

        s -= itab
        str_binding += s*" " + "end " + str_declaration + "\n"

        return str_binding

    @staticmethod
    def write_function( function ):
        """Generate a wrapper for a function.
           void functions in C will be called as subroutines,
           functions in C will be turned to subroutines by appending
           the return value as the last argument."""

        fname   = function['name']
        fname_f = fname + "_f08"
        fname_c = fname + "_f2c"
        rettype = function['rettype']
        symbol  = function['symbol']

        # Initialize a string with the fortran interface
        s = 0
        #TODOstr_function = s*" " + symbol + " " + fname_f + "("
        str_function = "\n" + s*" " + "subroutine" + " " + fname_f + "("
        initial_len = len( str_function ) - 1
        s += itab

        str_function += print_function_parameters( s, function, initial_len, True, True )

        str_function += s*" " + "use :: pastixf_interfaces, only : " + fname + "\n"
        str_function += s*" " + "use :: pastixf_bindings,   only : " + fname_c + "\n"
        str_function += print_function_use( s, function, 'fside', False, len("pastixf_interfaces") )

        str_function += print_function_variable_list( s, function, False )
        str_function += "\n"
        str_auxiliary = print_auxiliary_variables( s, function )
        if len(str_auxiliary) > 0:
            str_function += str_auxiliary + "\n"
        str_auxiliary = print_auxiliary_init( s, function )
        if len(str_auxiliary) > 0:
            str_function += str_auxiliary + "\n"
        str_function += print_function_call( s, function )
        str_auxiliary = print_auxiliary_fini( s, function )
        if len(str_auxiliary) > 0:
            str_function += str_auxiliary
            str_function += "\n"

        s -= itab
        #TODOstr_function = s*" " + "end " + symbol + " " + fname_f + "("
        str_function += s*" " + "end subroutine " + fname_f + "\n"

        return str_function

    @staticmethod
    def write_cbinding( function ):
        """Generate an interface for a function."""

        fname   = function['name']
        fname_f = fname + "_f08"
        fname_c = fname + "_f2c"
        rettype = function['rettype']
        symbol  = function['symbol']

        # Initialize a string with the fortran interface
        s = 0

        # Return type
        str_cbinding = "\n" + s*" " + rettype['type']
        if rettype['pointer'] > 0:
            str_cbinding += " " + rettype['pointer']*"*"
        str_cbinding += "\n"

        # Function call
        str_fcall = fname_c + "( "
        s = len( str_fcall )
        str_cbinding += str_fcall

        str_cbinding += print_function_parameters_c( s, function )
        str_cbinding += "{\n"
        s = 4

        str_cbinding += print_function_call_c( s, function )
        str_cbinding += "}\n"

        return str_cbinding

    @staticmethod
    def write_file( data, enum_list, struct_list, function_list ):
        """
        Generate a single python file. It will contains:
        enums, structs and interfaces of all C functions
        """

        modulefile = open( data['filename'], "w" )

        header_str = wrap_fortran.write_header( data )
        modulefile.write( header_str )

        # enums
        if (enum_list and len(enum_list) > 0):
            for enum in enum_list:
                enum_cpy = gen_enum_copy( enum )
                enum_str = wrap_fortran.write_enum( data, enum_cpy )
                modulefile.write( enum_str )

        # derived types
        if (struct_list and len(struct_list) > 0):
            for struct in struct_list:
                struct_str = wrap_fortran.write_struct( struct )
                modulefile.write( struct_str )

        footer_str = wrap_fortran.write_footer( data )
        modulefile.write( footer_str )

        modulefile.close()

        return data['filename']

    @staticmethod
    def write_interfaces( data, function_list ):
        """
        Generate a single python file. It will contains:
        enums, structs and interfaces of all C functions
        """

        # functions
        if (not function_list) or (len(function_list) < 1):
            return

        modulefile = open( data['filename'], "w" )

        header_str = wrap_fortran.write_header( data )
        modulefile.write( header_str )

        for function in function_list:
            function_str = wrap_fortran.write_interface( function )
            modulefile.write( function_str )

        footer_str = wrap_fortran.write_footer( data )
        modulefile.write( footer_str )

        modulefile.close()

        return data['filename']

    @staticmethod
    def write_bindings( data, function_list ):
        """
        Generate a single python file. It will contains:
        enums, structs and interfaces of all C functions
        """

        # functions
        if (not function_list) or (len(function_list) < 1):
            return

        modulefile = open( data['filename'], "w" )

        header_str = wrap_fortran.write_header( data )
        modulefile.write( header_str )

        for function in function_list:
            function_str = wrap_fortran.write_binding( function )
            modulefile.write( function_str )

        footer_str = wrap_fortran.write_footer( data )
        modulefile.write( footer_str )

        modulefile.close()

        return data['filename']

    @staticmethod
    def write_functions( data, function_list ):
        """
        Generate a single python file. It will contains:
        enums, structs and interfaces of all C functions
        """

        # functions
        if (not function_list) or (len(function_list) < 1):
            return

        modulefile = open( data['filename'], "w" )

        header_str = wrap_fortran.write_header( data, module=False )
        modulefile.write( header_str )

        for function in function_list:
            function_str = wrap_fortran.write_function( function )
            modulefile.write( function_str )

        footer_str = wrap_fortran.write_footer( data, module=False )
        modulefile.write( footer_str )

        modulefile.close()

        return data['filename']

    @staticmethod
    def write_cbindings( data, function_list ):
        """
        Generate a single python file. It will contains:
        enums, structs and interfaces of all C functions
        """

        # functions
        if (not function_list) or (len(function_list) < 1):
            return

        modulefile = open( data['filename'], "w" )

        header_str = wrap_fortran.write_header_c( data )
        modulefile.write( header_str )

        for function in function_list:
            function_str = wrap_fortran.write_cbinding( function )
            modulefile.write( function_str )

        footer_str = wrap_fortran.write_footer_c( data )
        modulefile.write( footer_str )

        modulefile.close()

        return data['filename']

    @staticmethod
    def write( enum_list, struct_list, function_list ):
        f = wrap_fortran.write_file( enums, enum_list, struct_list, None )
        print( "Exported file: " + f )

        for fun in function_list:
            wrap_fortran.prepare_function( fun )

        f = wrap_fortran.write_interfaces( interface, function_list )
        print( "Exported file: " + f )

        f = wrap_fortran.write_functions( functions, function_list )
        print( "Exported file: " + f )

        f = wrap_fortran.write_bindings( bindings, function_list )
        print( "Exported file: " + f )

        f = wrap_fortran.write_cbindings( cbindings, function_list )
        print( "Exported file: " + f )
