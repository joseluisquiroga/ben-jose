

/*************************************************************

This file is part of ben-jose.

ben-jose is free software: you can redistribute it and/or modify
it under the terms of the version 3 of the GNU General Public 
License as published by the Free Software Foundation.

ben-jose is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ben-jose.  If not, see <http://www.gnu.org/licenses/>.

------------------------------------------------------------

Copyright (C) 2011, 2014. QUIROGA BELTRAN, Jose Luis.
Id (cedula): 79523732 de Bogota - Colombia.
email: joseluisquirogabeltran@gmail.com

------------------------------------------------------------

central.cpp  

funcs that implement top level funcs.

--------------------------------------------------------------*/

#include "support.h"
#include "dimacs.h"
#include "brain.h"

//============================================================
// code for support

void
do_cnf_file()
{
	skeleton_glb& the_skl = GLB().gg_skeleton;
	instance_info& inst_info = GLB().get_curr_inst();
	brain the_brain(the_skl, inst_info);
	the_brain.solve_instance();
	GLB().count_instance(inst_info);
}


