

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

Copyright (C) 2007-2012, 2014-2016. QUIROGA BELTRAN, Jose Luis.
Id (cedula): 79523732 de Bogota - Colombia.
See https://github.com/joseluisquiroga/ben-jose

ben-jose is free software thanks to The Glory of Our Lord 
	Yashua Melej Hamashiaj.
Our Resurrected and Living, both in Body and Spirit, 
	Prince of Peace.

------------------------------------------------------------

batch_solver.h

Declaration of classes that batch solving.

--------------------------------------------------------------*/

#ifndef BATCH_SOLVER_H
#define BATCH_SOLVER_H

//=================================================================
// configuration defs

#define DO_FINAL_GETCHAR
#define PRINT_TOTALS_PERIOD 		10.0

//=================================================================

#include "tools.h"
#include "util_funcs.h"
#include "ben_jose.h"


#define BATCH_CK(prm) DBG_CK(prm)

#define LOG_NM_ERROR	"error.log"
#define LOG_NM_RESULTS	"results.log"
#define LOG_NM_STATS	"stats.log"
#define LOG_NM_ASSIGS	"assigs.log"
#define TEST_SUF		".test"

#define INVALID_TEST_FILE	"invalid_test_file"

//=================================================================
// pre-configuration decl

#define DO_PRINTS(prm)		prm

#define PRT_OUT_0(comm) \
	DO_PRINTS( \
		{ \
			bj_ostream& os = bj_out; \
			comm; \
			os.flush(); \
		} \
	) \
			
//--end_of_def

#define PRT_OUT_1(comm) /**/

//=================================================================
// aux funcs

inline
ch_string
get_log_name(ch_string f_nam, ch_string sufix){
	ch_string lg_nm = f_nam + "_" + sufix;
	return lg_nm;
}

//=================================================================
// decl
	
class batch_entry;

DECLARE_PRINT_FUNCS(batch_entry)

//=================================================================
// batch_entry

#define cbool_to_str(vv) ((vv == 0)?("0"):("1"))

class batch_entry {
public:
	ch_string	be_ff_nam;
	bj_output_t	be_out;
	ch_string	be_result_str;
	
	batch_entry(){
		be_ff_nam = "";
		bj_init_output(&be_out);
		be_result_str = "invalud_result_str";
	}
	
	~batch_entry(){
	}
	
	bj_ostream& 	print_batch_entry(bj_ostream& os, bool from_pt = false);
};

//=================================================================
// batch_solver

long		get_free_mem_kb();

class batch_solver {
public:
	bool			using_mem_ctrl;

	ch_string		help_str;
	ch_string		version_str;

	ch_string		result_titles_str;
	
	bool			op_only_deduc;
	bool			op_write_proof;
	bool			op_test_result;
	bool			op_test_save;

	bool			op_dbg_as_release;
	
	mem_size 		dbg_mem_at_start;

	bool			dbg_skip_print_info;
	
	bj_ostr_stream	error_stm;
	long			error_cod;

	ch_string		input_file_nm;

	bool			batch_log_on;
	ch_string		batch_name;
	ch_string		batch_log_errors_pth;
	ch_string		batch_log_results_pth;
	ch_string		batch_log_stats_pth;
	ch_string		batch_log_assigs_pth;

	integer			batch_num_files;
	integer			batch_consec;
	integer			batch_num_unknown_satisf;
	integer			batch_num_yes_satisf;
	integer			batch_num_no_satisf;
	integer			batch_num_timeout;
	integer			batch_num_memout;
	integer			batch_num_error;

	avg_stat		batch_stat_lits;
	avg_stat		batch_stat_laps;
	avg_stat		batch_stat_solve_tm;
	avg_stat		batch_stat_mem_used;

	avg_stat		batch_stat_load_tm;
	avg_stat		batch_stat_num_cnf_saved;
	avg_stat		batch_stat_num_cnf_finds;
	avg_stat		batch_stat_quick_discards;

	double			batch_start_time;
	double			batch_end_time;
	timer			batch_prt_totals_timer;

	row<batch_entry>	batch_instances;
	row<batch_entry>	batch_test_entries;
	
	bool			batch_test_has_errors;

	ch_string		gg_file_name;

	ch_string 		bc_slvr_path;
	bj_solver_t 	bc_solver;

	void 		init_batch_solver();
	void 		finish_batch_solver();

	batch_solver();
	~batch_solver();

	void		reset_global(){
		reset_err_msg();
		error_cod = -1;
	}

	bool		get_args(int argc, char** argv);
	void		set_input_name();

	batch_entry&	get_curr_inst(){
		long batch_idx = batch_consec - 1;
		DBG_CK(batch_instances.is_valid_idx(batch_idx));
		batch_entry& the_ans = batch_instances[batch_idx];
		return the_ans;
	}

	bool		in_valid_inst(){
		long batch_idx = batch_consec - 1;
		return (batch_instances.is_valid_idx(batch_idx));
	}

	ch_string	init_log_name(ch_string sufix);

	ch_string	get_file_name(bool& is_batch){
		ch_string f_nam = gg_file_name;
		is_batch = false;
		if(batch_name.size() > 0){
			is_batch = true;
			f_nam = batch_name;
		}
		return f_nam;
	}

	void	reset_err_msg(){
		error_stm.clear();
		error_stm.str() = "";
		error_stm.flush();
	}

	double	mem_percent_used();

	void	print_final_assig();
	void	count_instance(batch_entry& inst_info);

	bj_ostream&	print_mini_stats(bj_ostream& os);
	bj_ostream& 	print_stats(bj_ostream& os, double current_secs = 0.0);

	bj_ostream&	print_mem_used(bj_ostream& os);
	bj_ostream&	print_totals(bj_ostream& os, double curr_tm = 0.0);
	bj_ostream&	print_final_totals(bj_ostream& os);
	void		print_batch_consec();

	void	log_error_message(const ch_string& msg_log);
	void	log_batch_info(ch_string& log_nm);
	void	print_end_msg();
	void	read_batch_file(row<batch_entry>& names);
	void	work_all_instances();
	void	do_all_instances();
	void	do_cnf_file();

	ch_string	get_test_file_path();
	void	read_test_file(row<batch_entry>& test_entries, ch_string& file_nm_str);
	void	test_result_entries();
	bool	test_entry(batch_entry& rr, batch_entry& tt);
};

//=================================================================
// print functions

inline
bj_ostream& 	
batch_entry::print_batch_entry(bj_ostream& os, bool from_pt){
	os << be_result_str;
	return os;
}

DEFINE_PRINT_FUNCS(batch_entry)

//=================================================================
// global functions

void	chomp_string(ch_string& s1);
int		solver_main(int argc, char** argv);


#endif		// BATCH_SOLVER_H

