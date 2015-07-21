

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

Copyright (C) 2011, 2014-2015. QUIROGA BELTRAN, Jose Luis.
Id (cedula): 79523732 de Bogota - Colombia.
email: joseluisquirogabeltran@gmail.com

ben-jose is free software thanks to The Glory of Our Lord 
	Yashua Melej Hamashiaj.
Our Resurrected and Living, both in Body and Spirit, 
	Prince of Peace.

------------------------------------------------------------

analyser.cpp  

analyser class methos and funcs.

--------------------------------------------------------------*/

#include "brain.h"
#include "solver.h"
#include "dbg_strings_html.h"

//============================================================
// static vars

neuromap*	analyser::NULL_NEUROMAP = NULL_PT;

//============================================================
// aux funcs

long	set_spots_of(brain& brn, row_neuron_t& neus){
	long num_neu = 0;
	for(long ii = 0; ii < neus.size(); ii++){
		neuron* neu = neus[ii];
		if(! neu->ne_spot){
			neu->set_spot(brn);
			num_neu++;
		}
	}
	return num_neu;
}

long	reset_spots_of(brain& brn, row_neuron_t& neus){
	long num_neu = 0;
	for(long ii = 0; ii < neus.size(); ii++){
		neuron* neu = neus[ii];
		if(neu->ne_spot){
			neu->reset_spot(brn);
			num_neu++;
		}
	}
	return num_neu;
}

void
brain::deactivate_last_map(){
	BRAIN_CK(! br_maps_active.is_empty());
	neuromap* pt_mpp = br_maps_active.last();
	BRAIN_CK(pt_mpp != NULL_PT);
	neuromap& mpp = *pt_mpp;

	DBG_PRT(13, os << "DEACTIVATING NEUROMAP=" << (void*)(&(mpp)));

	mpp.map_deactivate();
}

void
analyser::reset_deduc(){
	de_all_noted.clear(true, true);
	de_all_propag.clear(true, true);
	
	de_all_learned_forced.clear();
	de_all_learned_propag.clear();
	
	de_next_bk_psig.init_prop_signal();
	BRAIN_CK(! found_learned());
}

void
analyser::init_analyser(brain* brn){
	de_brain = brn;

	if(brn != NULL_PT){
		de_ref.init_qlayers_ref(&(brn->br_charge_trail));
	}
	de_all_confl.clear(true, true);
	de_max_ti = INVALID_TIER;
	
	de_trail_propag_not_sel_neus.clear_all_neurons();
	de_forced_not_sel_neus.clear_all_neurons();
	de_propag_not_sel_neus.clear_all_neurons();

	de_tmp_neuromap = NULL_PT;
	
	reset_deduc();
}

void
analyser::init_nk_with_note0(notekeeper& nkpr, brain& brn){
	long* pt_tot_note0 = NULL_PT;
	pt_tot_note0 = &(brn.br_qu_tot_note0);
	nkpr.init_notekeeper(&brn, INVALID_LEVEL);
	nkpr.init_funcs(pt_tot_note0, &quanton::has_note0, 
							   &quanton::set_note0, &quanton::reset_its_note0, 
								&set_all_note0, &reset_all_its_note0, 
								&append_all_not_note0, &same_quantons_note0
   				);
	
	BRAIN_CK(nkpr.nk_get_counter() == 0);
}

void
analyser::init_nk_with_note5(notekeeper& nkpr, brain& brn){
	long* pt_tot_note5 = NULL_PT;
	pt_tot_note5 = &(brn.br_qu_tot_note5);
	nkpr.init_notekeeper(&brn, INVALID_LEVEL);
	nkpr.init_funcs(pt_tot_note5, &quanton::has_note5, 
							   &quanton::set_note5, &quanton::reset_its_note5, 
								&set_all_note5, &reset_all_its_note5, 
								&append_all_not_note5, &same_quantons_note5
   				);
	BRAIN_CK(nkpr.nk_get_counter() == 0);
}

bool
ck_ord_last_tiers(brain& brn, row<prop_signal>& all_noted){
#ifdef FULL_DEBUG
	if(all_noted.size() < 2){
		return true;
	}
	BRAIN_DBG(
		long cur_idx = all_noted.last_idx();
		long prv_idx = cur_idx - 1;
		prop_signal& cur_ps = all_noted[cur_idx];
		prop_signal& prv_ps = all_noted[prv_idx];
		long c_ti = cur_ps.ps_tier;
		long p_ti = prv_ps.ps_tier;
	)
	BRAIN_CK_PRT((c_ti <= p_ti), os << "____________\n" ;
		ch_string ff = brn.dbg_prt_margin(os); os << ff << bj_eol;
		all_noted.print_row_data(os, true, "\n");
		brn.print_trail(os);
	);
#endif
	return true;
}

void
analyser::set_nxt_noted(quanton* pt_nxt_qua){
	if(pt_nxt_qua == NULL_PT){
		de_next_bk_psig.init_prop_signal();
		return;
	}
	BRAIN_CK(pt_nxt_qua != NULL_PT);
			
	quanton& nxt_qua = *pt_nxt_qua;
	BRAIN_CK(nxt_qua.qu_id != 0);
	BRAIN_CK(nxt_qua.is_pos());
	
	notekeeper& nkpr = de_nkpr;
	if(! nkpr.nk_has_note(nxt_qua)){
		BRAIN_CK(false);
		return;
	}
	
	long qlv = nxt_qua.qlevel();
	nkpr.update_notes_layer(qlv);
	
	row<prop_signal>& all_noted = de_all_noted;
	
	de_next_bk_psig.init_qua_signal(nxt_qua);
	
	prop_signal& nxt_sig = all_noted.inc_sz();
	nxt_sig = de_next_bk_psig;
	if((nxt_sig.ps_source != NULL_PT) && ! nxt_sig.ps_source->ne_original){
		BRAIN_CK(ck_end_of_lrn_nmp());
		de_all_learned_forced.push(pt_nxt_qua);
		nxt_sig.ps_source = NULL_PT;
	}
	BRAIN_CK(ck_ord_last_tiers(get_de_brain(), all_noted));

	nkpr.nk_reset_note(nxt_qua);
	BRAIN_CK(nxt_qua.qlevel() == nkpr.dk_note_layer);
	nkpr.dec_notes();

	BRAIN_CK_PRT((pt_nxt_qua == last_qu_noted()),
			os << "cur_qu=" << de_ref.get_curr_quanton()
			<< " l_qu_not=" << last_qu_noted()
	);
}

void
analyser::set_nxt_propag(quanton* pt_nxt_qua){
	if(pt_nxt_qua == NULL_PT){
		return;
	}
	quanton& nxt_qua = *pt_nxt_qua;
	
	prop_signal& nxt_sig = de_all_propag.inc_sz();
	nxt_sig.init_qua_signal(nxt_qua);
	
	if((nxt_sig.ps_source != NULL_PT) && ! nxt_sig.ps_source->ne_original){
		de_all_learned_propag.push(&nxt_qua);
		nxt_sig.ps_source = NULL_PT;
	}
	
}

bool
analyser::find_next_noted(bool stop_at_nmp_ends){
	qlayers_ref& qlr = de_ref;
	notekeeper& nkpr = de_nkpr;
	quanton* nxt_qua = qlr.get_curr_quanton();
	
	bool in_end = false;
	while(nxt_qua != NULL_PT){
		nxt_qua = qlr.get_curr_quanton();
		if(nxt_qua == NULL_PT){
			de_next_bk_psig.init_prop_signal();
			break;
		}
		if(nkpr.nk_has_note(*nxt_qua)){
			set_nxt_noted(nxt_qua);
			break; 
		}
		
		quanton* curr_qu = qlr.dec_curr_quanton();
		set_nxt_propag(curr_qu);
		
		if(stop_at_nmp_ends && (curr_qu != NULL_PT) && curr_qu->is_qu_end_of_nmp()){
			if(nkpr.nk_has_note(*curr_qu)){
				set_nxt_noted(curr_qu);
			}
			in_end = true;
			break;
		}
	}

	BRAIN_CK(nkpr.dk_note_layer <= get_de_brain().level());
	return in_end;
}

bool
analyser::find_next_source(bool only_origs, bool stop_at_nmp_ends)
{
	BRAIN_DBG(notekeeper& nkpr = de_nkpr);
	
	neuron*& nxt_src = de_next_bk_psig.ps_source;
	
	//bool only_origs = (! is_null_neuromap(to_update));

	if((nxt_src != NULL) && (! only_origs || nxt_src->ne_original)){
		set_notes_of(nxt_src->ne_fibres, false);
	}

	BRAIN_CK_PRT((only_origs || (nkpr.dk_tot_noted > 0)), 
				 os << "________"  << bj_eol;
				 os << "tot_no=" << nkpr.dk_tot_noted << bj_eol;
				 os << "cur_qu=" << de_ref.get_curr_quanton() << bj_eol;
				 os << "cur_qlv=" << de_ref.get_curr_qlevel());

	bool in_end = find_next_noted(stop_at_nmp_ends);
	return in_end;
}

void
analyser::set_notes_of(row_quanton_t& causes, bool is_first){
	notekeeper& nkpr = de_nkpr;
	BRAIN_CK(! causes.is_empty());
	BRAIN_CK(is_first || (causes[0]->get_charge() == cg_positive) );
	BRAIN_CK(! is_first || (causes[0]->get_charge() == cg_negative) );

	long from = (is_first)?(0):(1);
	long until = causes.size();
	nkpr.set_motive_notes(causes, from, until);
}

bool
analyser::ck_deduc_init(long deduc_lv, bool full_ck){
#ifdef FULL_DEBUG
	BRAIN_DBG(
		qlayers_ref& qlr = de_ref;
		notekeeper& nkpr = de_nkpr;
		brain& brn = get_de_brain();
		bool in_root = (brn.level() == ROOT_LEVEL);
	)
	BRAIN_CK(nkpr.nk_get_counter() == 0);
	
	long num_propag = (full_ck && qlr.has_curr_quanton()) ? (1) : (0);
	
	BRAIN_CK(nkpr.dk_quas_lyrs.is_empty());
	BRAIN_CK(de_nkpr.nk_get_counter() == 0);
	BRAIN_CK(de_all_noted.size() == 0); 
	BRAIN_CK_PRT((de_all_propag.size() == num_propag), os << "________________\n"; 
		brn.dbg_prt_margin(os);
		os << " full_ck=" << full_ck;
		os << " prop_sz=" << de_all_propag.size();
		os << " has_curr_qu=" << qlr.has_curr_quanton();
		os << " num_propag=" << num_propag;
	);
	BRAIN_CK(de_all_learned_forced.size() == 0); 
	BRAIN_CK(de_all_learned_propag.size() == 0); 
	
	BRAIN_CK(! full_ck || in_root || (nkpr.dk_note_layer > 0));
	BRAIN_CK(! full_ck || (deduc_lv != INVALID_LEVEL));
	BRAIN_CK(! full_ck || (nkpr.dk_note_layer == deduc_lv));
	BRAIN_CK(! full_ck || (qlr.has_curr_quanton()));
#endif
	return true;
}

void	
analyser::deduction_init(row_quanton_t& causes){
	qlayers_ref& qlr = de_ref;
	notekeeper& nkpr = de_nkpr;
	
	BRAIN_CK(causes.size() > 1);

	reset_deduc();
	BRAIN_CK(ck_deduc_init(INVALID_LEVEL, false));
	
	long deduc_lv = find_max_level(causes);
	
	nkpr.init_notes(deduc_lv);
	qlr.reset_curr_quanton();
	
	set_nxt_propag(qlr.get_curr_quanton());

	BRAIN_CK(ck_deduc_init(deduc_lv, true));
	
	set_notes_of(causes, true);
	
	BRAIN_CK_PRT((nkpr.dk_tot_noted > 0), get_de_brain().dbg_prt_margin(os));
	
	find_next_noted();
}

row_quanton_t&
analyser::get_all_causes(row_quanton_t& all_quas){
	brain& brn = get_de_brain();
	
	row_neuron_t& all_cfl = brn.br_tmp_all_cfl;
	all_cfl.clear();
	append_all_trace_neus(de_all_confl, all_cfl);
	
	all_quas.clear();
	get_quas_of(brn, all_cfl, all_quas);
	return all_quas;
}

void
analyser::init_calc_nmp(long min_lv){
	brain& brn = get_de_brain();
	row_quanton_t& all_quas = brn.br_tmp_all_causes;

	get_all_causes(all_quas);
	
	de_max_ti = find_max_tier(all_quas);
		
	deduction_init(all_quas); // multi_confl
	//deduction_init(get_first_causes());
	
	BRAIN_CK(de_all_confl.first().get_level() == get_de_brain().level());
	BRAIN_CK(get_de_brain().br_ne_tot_tag1 == 0);
	BRAIN_CK(min_lv < de_ref.get_curr_qlevel());
	
	de_trail_propag_not_sel_neus.clear_all_neurons();
	de_forced_not_sel_neus.clear_all_neurons();
	de_propag_not_sel_neus.clear_all_neurons();

	de_tmp_neuromap = NULL_PT;
}

void
analyser::deduction_analysis(row_quanton_t& causes, deduction& dct){
	qlayers_ref& qlr = de_ref;
	MARK_USED(qlr);
	notekeeper& nkpr = de_nkpr;
	
	deduction_init(causes);
	while(! is_end_of_dct()){
		BRAIN_CK(qlr.has_curr_quanton());
		find_next_source();
	}
	BRAIN_CK(qlr.has_curr_quanton());

	fill_dct(dct);
	
	// reset all
	nkpr.clear_all_quantons();
	BRAIN_CK(nkpr.dk_quas_lyrs.is_empty());
	BRAIN_CK(nkpr.nk_get_counter() == 0);
}

void
analyser::fill_dct(deduction& dct){
	qlayers_ref& qlr = de_ref;
	notekeeper& nkpr = de_nkpr;
	brain& brn = get_de_brain();
	
	quanton* pt_qua = qlr.get_curr_quanton();
	BRAIN_CK(pt_qua != NULL_PT);
	
	quanton& qua = *pt_qua;
	quanton& opp = qua.opposite(); 

	BRAIN_CK(is_end_of_dct());
	BRAIN_CK(opp.qlevel() == nkpr.dk_note_layer);
	BRAIN_CK(opp.get_charge() == cg_negative);

	dct.init_deduction();
	dct.dt_motives.set_cap(nkpr.dk_tot_noted + 1);

	nkpr.dk_quas_lyrs.get_all_ordered_quantons(dct.dt_motives);

	dct.dt_target_level = find_max_level(dct.dt_motives);

	BRAIN_CK(! nkpr.nk_has_note(opp));
	dct.dt_forced = &opp;
	
	bool br_in_root = (brn.level() == ROOT_LEVEL);

	BRAIN_CK(br_in_root || (dct.dt_target_level < nkpr.dk_note_layer));
	BRAIN_CK(ck_motives(brn, dct.dt_motives));

	DBG_PRT(20, os << "find_dct of deduction=" << dct);
	
	if(br_in_root){
		BRAIN_CK(dct.dt_target_level == ROOT_LEVEL);
		BRAIN_CK(dct.dt_motives.is_empty());
		dct.dt_target_level = INVALID_LEVEL;
	}
	
}

bool
analyser::ck_end_of_lrn_nmp(){
#ifdef FULL_DEBUG
	BRAIN_CK(de_next_bk_psig.ps_source != NULL_PT);
	BRAIN_CK(! de_next_bk_psig.ps_source->ne_original);
	
	BRAIN_DBG(
		brain& brn = get_de_brain();
		qlayers_ref& qlr = de_ref;
		quanton* qua = de_next_bk_psig.ps_quanton;
	)
	
	BRAIN_CK(qua != NULL_PT);
	BRAIN_CK(qua == qlr.get_curr_quanton());
	
	BRAIN_DBG(
		long qti = qua->qu_tier;
		long num_quas = brn.br_charge_trail.get_qu_layer(qti).size();

		long q_lv = qua->qlevel();
		bool has_lnd = brn.get_data_level(q_lv).has_learned();
	)
	BRAIN_CK(num_quas == 1);
	BRAIN_CK(has_lnd);
#endif	
	return true;
}

void
neuromap::set_min_ti_max_ti(){
	BRAIN_CK(na_orig_cho != NULL_PT);
	BRAIN_REL_CK(na_orig_cho != NULL_PT);
	BRAIN_CK(na_min_ti == INVALID_TIER);
	BRAIN_CK(na_max_ti == INVALID_TIER);
	BRAIN_CK(na_orig_cho->qu_tier != INVALID_TIER);
	
	na_min_ti = na_orig_cho->qu_tier;
	
	if(has_submap()){
		quanton* nxt_cho = na_submap->na_orig_cho;
		BRAIN_CK(nxt_cho != NULL_PT);
		BRAIN_CK(nxt_cho->qu_tier != INVALID_TIER);
		
		na_max_ti = nxt_cho->qu_tier - 1;
	} else {
		brain& brn = get_brn();
		na_max_ti = brn.tier();
	}
	BRAIN_CK(na_min_ti != INVALID_TIER);
	BRAIN_CK(na_max_ti != INVALID_TIER);
	BRAIN_CK(na_min_ti <= na_max_ti);
}

neuromap*
analyser::update_neuromap(neuromap* sub_nmp){
	qlayers_ref& qlr = de_ref;
	BRAIN_CK(qlr.is_end_of_nmp());
	
	brain& brn = get_de_brain();

	neuromap& nxt_nmp = brn.locate_neuromap();
	
	BRAIN_DBG(nxt_nmp.na_dbg_update_tk.update_ticket(brn));
	
	nxt_nmp.na_submap = sub_nmp;
	BRAIN_DBG(
		if(sub_nmp != NULL_PT){ 
			nxt_nmp.na_dbg_num_submap = sub_nmp->na_dbg_num_submap + 1;
		}
	);
	
	BRAIN_CK(! nxt_nmp.has_submap() || nxt_nmp.na_all_confl.is_empty());
	
	quanton* qua = qlr.get_curr_quanton();
	BRAIN_CK(qua != NULL_PT);
	BRAIN_REL_CK(qua != NULL_PT);
	
	nxt_nmp.na_orig_lv = qua->qlevel();
	nxt_nmp.na_orig_ti = qua->qu_tier;
	nxt_nmp.na_orig_ki = qua->get_cy_kind();
	nxt_nmp.na_orig_cho = qua;
	
	if(qua->opposite().is_mono()){
		BRAIN_CK(nxt_nmp.na_submap != NULL_PT);
		nxt_nmp.na_nxt_no_mono = nxt_nmp.na_submap->na_nxt_no_mono;
		BRAIN_CK(nxt_nmp.is_na_mono());
	}
	
	BRAIN_CK(nxt_nmp.na_trail_propag.is_empty());
	BRAIN_CK(nxt_nmp.na_forced.is_empty());
	
	if(sub_nmp == NULL_PT){
		BRAIN_CK(! nxt_nmp.is_na_mono());
		BRAIN_CK(! de_all_confl.is_empty());
		de_all_confl.copy_to(nxt_nmp.na_all_confl);  // multi_confl
	}
	
	de_all_propag.move_to(nxt_nmp.na_trail_propag);
	de_all_noted.move_to(nxt_nmp.na_forced);  // full_prep
	
	nxt_nmp.set_min_ti_max_ti();

	// set all filled
	
	nxt_nmp.set_all_filled_by_forced();
	nxt_nmp.map_forced_set_all_note2_n_tag2();
	
	//BRAIN_CK(brn.br_qu_tot_note2 == 0); // full_prep
	
	nxt_nmp.set_propag(); // full_nmp
	nxt_nmp.set_all_filled_by_propag(); // full_nmp
	nxt_nmp.map_propag_set_all_note3_n_tag3(); // full_nmp
	
	BRAIN_CK(nxt_nmp.map_ck_all_quas());
	//BRAIN_CK(brn.br_qu_tot_note2 == 0); // full_prep

	//BRAIN_CK(nxt_nmp.na_propag.is_empty());
	//BRAIN_CK(nxt_nmp.na_all_filled_by_propag.is_empty());
	
	// set covs
	nxt_nmp.map_fill_cov_by_forced(de_forced_not_sel_neus);
	nxt_nmp.map_fill_cov_by_propag(de_propag_not_sel_neus);
	
	return &nxt_nmp;
}

bool
analyser::found_learned(){
	//bool fl = found_learned_forced(); 
	bool fl = found_learned_propag(); // full_nmp
	return fl;
}

quanton&
analyser::first_learned(){
	BRAIN_CK(found_learned());
	//row_quanton_t& lrnd = de_all_learned_forced;
	row_quanton_t& lrnd = de_all_learned_propag; // full_nmp
	
	BRAIN_CK(lrnd.first() != NULL_PT);
	quanton& qua = *(lrnd.first());
	return qua;
}


bool
brain::ck_cov_flags(){
#ifdef FULL_DEBUG
	BRAIN_CK(br_qu_tot_note2 == 0);
	BRAIN_CK(br_ne_tot_tag2 == 0);
	BRAIN_CK(br_qu_tot_note3 == 0);
	BRAIN_CK(br_ne_tot_tag3 == 0);
	BRAIN_CK(br_ne_tot_tag4 == 0);
#endif
	return true;
}

bool 
neuromap::map_lv_already_has_setup(){
	return map_get_data_level().has_setup_neuromap();
}

neuromap*
analyser::calc_neuromap(long min_lv, neuromap* prev_nmp, bool with_lrnd, bool in_setup)
{
	BRAIN_DBG(brain& brn = get_de_brain());
	//BRAIN_CK(brn.ck_cov_flags());
	
	qlayers_ref& qlr = de_ref;
	
	//neuromap*& out_nmp = de_tmp_neuromap;
	neuromap* out_nmp = prev_nmp;
	
	BRAIN_CK(brn.br_tot_qu_marks == 0);
	BRAIN_CK(brn.br_tot_ne_spots == 0);
	
	BRAIN_CK(brn.br_data_levels.is_valid_idx(min_lv));
	if(out_nmp == NULL_PT){
		init_calc_nmp(min_lv);
	}
	if(! with_lrnd && found_learned()){
		return out_nmp;
	}
	BRAIN_CK(with_lrnd || ! found_learned());
	if(in_setup && (out_nmp != NULL_PT)){
		if(out_nmp->map_lv_already_has_setup()){
			return out_nmp;
		}
	}
	
	BRAIN_DBG(
		bool just_updated = false;
		if(qlr.get_curr_qlevel() <= min_lv){
			just_updated = true;
		}
	);
	
	DBG_PRT(38, os << "=========================================================\n";
			os << "clc_nmp{ " << "cur_qu=" << de_ref.get_curr_quanton()
			<< " tot_no=" << de_nkpr.dk_tot_noted
			<< " min_lv=" << min_lv 
			<< " cur_qlv=" << qlr.get_curr_qlevel() << bj_eol
			<< " out_nmp=" << out_nmp 
			<< "}");

	BRAIN_DBG(long num_loop = 0);
	while(qlr.get_curr_qlevel() > min_lv){
		BRAIN_DBG(num_loop++);
		BRAIN_DBG(just_updated = false);
		
		bool in_end = find_next_source(true, true);
		if(in_end){
			BRAIN_DBG(just_updated = true);
			if(! with_lrnd && found_learned()){
				break;
			}
			BRAIN_CK(! in_setup || ! found_learned());
			
			out_nmp = update_neuromap(out_nmp);
			BRAIN_CK(out_nmp != NULL_PT);
			BRAIN_DBG(out_nmp->na_dbg_nxt_lv = min_lv);
			
			if(in_setup){
				DBG_PRT(65, os << " out_nmp=" << out_nmp);
				BRAIN_CK(! found_learned());
				if(! out_nmp->is_na_mono()){
					DBG_PRT(65, os << " make_dom nmp=" << out_nmp);
					out_nmp->map_make_nxt_monos_dominated();
					out_nmp->map_make_dominated();
				}
				if(out_nmp->map_lv_already_has_setup()){
					break;
				}
			}
			
			if(qlr.get_curr_qlevel() == (min_lv + 1)){
				break;
			}
		}
		BRAIN_DBG(if(qlr.get_curr_quanton() == NULL_PT){ just_updated = true; });
	}

	BRAIN_DBG(
		if(out_nmp != NULL_PT){
			BRAIN_CK_PRT(just_updated, os << "___________\n";
					brn.dbg_prt_margin(os); 
					os << " curr_qu=" << qlr.get_curr_quanton() << "\n";
					os << " curr_lv=" << qlr.get_curr_qlevel() << "\n";
					os << " min_lv=" << min_lv << "\n";
					os << " out_nmp=" << out_nmp
			);
			BRAIN_CK(out_nmp->dbg_ck_all_neus());
			BRAIN_CK(out_nmp->dbg_ck_all_confl_tag1());
		}
	);
	BRAIN_CK(brn.br_tot_qu_marks == 0);
	BRAIN_CK(brn.br_tot_ne_spots == 0);
	//BRAIN_CK(brn.ck_cov_flags());

	DBG_PRT(38, os << "CLC_NMP{" 
			<< " num_loop=" << num_loop
			<< " min_lv=" << min_lv
			<< " cur_qu=" << de_ref.get_curr_quanton()
			<< " cur_qlv=" << qlr.get_curr_qlevel() << bj_eol
			<< " nxt_ps=" << de_next_bk_psig
			<< " out_nmp=" << out_nmp << bj_eol
			<< " eonmp=" << qlr.is_end_of_nmp()
			<< "}");
	
	BRAIN_CK((out_nmp == NULL_PT) || ! out_nmp->is_active());
	
	BRAIN_CK(with_lrnd || (out_nmp == NULL_PT) || out_nmp->map_ck_orig());
	BRAIN_CK(with_lrnd || (out_nmp == NULL_PT) || ! out_nmp->na_is_head);
	
	/*
	if(! with_lrnd){
		BRAIN_DBG(neuromap* dbg_in = out_nmp);
		BRAIN_CK((out_nmp == NULL_PT) || ! out_nmp->na_is_head);
		out_nmp = calc_orig_neuromap(out_nmp);
		BRAIN_CK((out_nmp == NULL_PT) || ! out_nmp->na_is_head);
		BRAIN_CK(out_nmp == dbg_in);
	}*/
	return out_nmp;
}

void
analyser::end_analysis(){
	//BRAIN_DBG(brain& brn = get_de_brain());
	notekeeper& nkpr = de_nkpr;
	nkpr.clear_all_quantons();
	BRAIN_CK(nkpr.dk_quas_lyrs.is_empty());
	BRAIN_CK(nkpr.nk_get_counter() == 0);	
}

void
get_quas_of(brain& brn, row_neuron_t& all_neus, row_quanton_t& all_quas){
	BRAIN_CK(brn.br_qu_tot_note1 == 0);
	all_quas.clear();
	for(long aa = 0; aa < all_neus.size(); aa++){
		BRAIN_CK(all_neus[aa] != NULL_PT);
		neuron& neu = *(all_neus[aa]);
		append_all_not_note1(brn, neu.ne_fibres, all_quas);
		set_all_note1(brn, neu.ne_fibres);
	}
	reset_all_note1(brn, all_quas);
	BRAIN_CK(brn.br_qu_tot_note1 == 0);
}

long
analyser::find_min_lv_to_setup(long tg_lv){
	brain& brn = get_de_brain();
	row<leveldat*>& all_lv = brn.br_data_levels;
	MARK_USED(all_lv);
	
	BRAIN_CK(brn.level() == all_lv.last_idx());
	BRAIN_CK(all_lv.is_valid_idx(tg_lv));
	
	long min_lv = INVALID_LEVEL;
	for(int aa = tg_lv; aa > 0; aa--){
		leveldat& lv = brn.get_data_level(aa);

		if(lv.has_setup_neuromap()){
			break;
		}
		min_lv = aa;
	}
	return min_lv;
}

bool
brain::needs_lv_setup(long nxt_lv, neuromap* in_nmp){
	bool nds = ((in_nmp != NULL_PT) && ! lv_has_setup_nmp(nxt_lv + 1));
	return nds;
}

void
brain::make_lv_last_active(long nxt_lv){
	while(! br_maps_active.is_empty()){
		neuromap* nmp = br_maps_active.last();
		BRAIN_CK(nmp != NULL_PT);
		long lv_act = nmp->map_get_active_lv();
		if(lv_act <= nxt_lv){
			break;
		}
		deactivate_last_map();
	}
}

neuromap* 
find_parent_of(neuromap& hd_nmp, neuromap& sb_nmp){
	neuromap* pnt_nmp = NULL_PT;
	neuromap* tgt_nmp = &sb_nmp;
	neuromap* nxt_nmp = &hd_nmp;
	while(nxt_nmp != NULL_PT){
		neuromap* sub_nmp = nxt_nmp->na_submap;
		if(sub_nmp == tgt_nmp){
			pnt_nmp = nxt_nmp;
			break;
		}
		nxt_nmp = nxt_nmp->na_submap;
	}
	return pnt_nmp;
}

void
brain::cut_old_head_of(neuromap& nmp, long old_rl_sz){
	BRAIN_CK(old_rl_sz >= 0);
	BRAIN_CK(! nmp.is_active());
	BRAIN_CK(! nmp.na_is_head);
	
	long nmp_oact_idx = nmp.na_old_active_idx;
	for(long aa = old_rl_sz; aa < br_nmps_to_release.size(); aa++){
		if(br_nmps_to_release[aa] == NULL_PT){
			continue;
		}
		neuromap& nmp_rl = *(br_nmps_to_release[aa]);
		BRAIN_CK(! nmp_rl.is_active());
		if(nmp_rl.na_old_active_idx < nmp_oact_idx){
			BRAIN_CK(nmp_rl.na_is_head);
			neuromap* pnt_nmp = find_parent_of(nmp_rl, nmp);
			BRAIN_CK(pnt_nmp != NULL_PT);
			BRAIN_CK(pnt_nmp->na_submap == &nmp);
			nmp_rl.map_remove_from_release();
			neuromap* tmp_nmp = cut_neuromap(&nmp_rl, pnt_nmp);
			MARK_USED(tmp_nmp);
			BRAIN_CK(tmp_nmp == &nmp);
			break;
		}
	}
}

void
brain::add_lv_neuromap_to_write(long nxt_lv, deduction& dct){
	quanton* dct_qu = dct.dt_forced;
	BRAIN_CK(dct_qu != NULL_PT);
	
	if(level() == ROOT_LEVEL){
		BRAIN_CK(nxt_lv == INVALID_LEVEL);
		return;
	}
	//long deduc_lv = (nxt_lv + 1);
	long deduc_lv = dct.get_deduc_lv();;
	BRAIN_CK(nxt_lv < deduc_lv);
	
	leveldat& lv_s = get_data_level(deduc_lv);
	if(lv_s.has_setup_neuromap()){
		leveldat& lv_ti = get_data_level(nxt_lv + 1);
		BRAIN_CK(lv_ti.ld_chosen != NULL_PT);
		long n_ti = lv_ti.ld_chosen->qu_tier;
		BRAIN_CK(n_ti >= 0);
		
		neuromap& nmp = lv_s.get_setup_neuromap();
		BRAIN_CK(nmp.is_active());
		
		long old_rl_sz = br_nmps_to_release.size();
		
		make_lv_last_active(nxt_lv);
		
		BRAIN_CK(br_nmps_to_release.size() >= old_rl_sz);
		if(! nmp.na_is_head && (br_nmps_to_release.size() > old_rl_sz)){
			cut_old_head_of(nmp, old_rl_sz);
		}
		
		leveldat& lv_w = get_data_level(nxt_lv);
	
		BRAIN_CK(! nmp.is_active());
		BRAIN_CK_PRT((nmp.map_can_activate()), os << "__________\n ABORT_DATA\n";
			os << " nmp=" << &nmp << "\n";
		);
		BRAIN_CK(nmp.na_mates.is_alone());
		
		BRAIN_CK(nmp.na_next_psig.is_ps_virgin());
		nmp.na_next_psig.ps_quanton = dct_qu;
		nmp.na_next_psig.ps_tier = n_ti;
		
		DBG_PRT(43, os << " COND_WRT_PSIG=" << nmp.na_next_psig << " dct=" << dct);
		
		lv_w.ld_nmps_to_write.bind_to_my_right(nmp.na_mates);
		nmp.map_activate(dbg_call_1);
		BRAIN_CK(nmp.is_active());
		BRAIN_CK(br_maps_active.last() == &nmp);
		
		BRAIN_DBG(nmp.na_dbg_st_lv = nxt_lv);
		
		DBG_PRT(50, os << "setup_to_wrt=" << &nmp << " w_lv=" << nxt_lv);
	} 
}

void
write_all_neuromaps(row<neuromap*>& to_wrt){
	for(int aa = 0; aa < to_wrt.size(); aa++){
		BRAIN_CK(to_wrt[aa] != NULL_PT);
		neuromap& nmp = *(to_wrt[aa]);
		nmp.map_write();
		
		BRAIN_DBG(
			long& max_ns = nmp.get_brn().br_dbg.dbg_max_wrt_num_subnmp;
			if(nmp.na_dbg_num_submap > max_ns){
				max_ns = nmp.na_dbg_num_submap;
			}
		);

		nmp.deactivate_until_me(true);
	}
	to_wrt.clear();
}

void
brain::analyse(row<prop_signal>& all_confl, deduction& out_dct){
	BRAIN_CK(ck_cov_flags());
	BRAIN_CK(ck_trail());
	BRAIN_CK(! all_confl.is_empty());
	//BRAIN_CK(all_confl.first().ps_tier >= tier());
	
	analyser& mper = br_neuromaper_anlsr;
	analyser& dedser = br_deducer_anlsr;
	
	mper.set_conflicts(all_confl);
	dedser.set_conflicts(all_confl);
	
	bool f_lrnd = false;
	
	DBG_PRT(40, os << "bef_ana=" << bj_eol; print_trail(os);
		os << " num_conf=" << br_all_conflicts_found.size() << " br_lv=" << level()
		<< " br_ti=" << tier();
	);
	
	row<neuromap*>& to_wrt = br_tmp_maps_to_write;
	to_wrt.clear();
	
	dedser.deduction_analysis(dedser.get_first_causes(), out_dct);
	DBG_PRT(40, os << HTMi_h1 << "out_dct=" << HTMe_h1 << out_dct);
	dedser.make_noted_dominated_and_deduced(to_wrt);
	//get_all_retracted_to_write(out_dct, to_wrt);
	f_lrnd = dedser.found_learned();

	BRAIN_DBG(deduction fst_dct; out_dct.copy_to_dct(fst_dct));
	BRAIN_CK(br_ne_tot_tag1 == 0);
	BRAIN_CK(br_qu_tot_note0 == 0);
	
	long tg_lv = out_dct.dt_target_level;
	
	DBG_PRT_COND(40, f_lrnd, os << "FOUND__LEARNED 1");
	
	deduction& fnd_dct = br_tmp_find_dct;
	long tg2 = tg_lv;
	neuromap* f_nmp = NULL_PT;
	if(! f_lrnd){
		BRAIN_CK(br_qu_tot_note0 == 0);
		f_nmp = mper.neuromap_find_analysis(dedser, tg2, fnd_dct, to_wrt);
		BRAIN_CK(f_nmp != NULL_PT);
		f_lrnd = mper.found_learned();
	}
	if(tg_lv != tg2){
		BRAIN_CK(f_nmp != NULL_PT);
		BRAIN_CK(! fnd_dct.is_dt_virgin());
		
		tg_lv = tg2;
		out_dct.init_deduction();
		fnd_dct.move_to_dct(out_dct);
		BRAIN_CK(tg_lv == out_dct.dt_target_level);
	}

	DBG_PRT_COND(40, f_lrnd, os << "FOUND__LEARNED 2");
	if(! f_lrnd){
		f_nmp = mper.neuromap_setup_analysis(tg_lv, f_nmp, out_dct); // activates new maps
	}
	
	add_lv_neuromap_to_write(tg_lv, out_dct);
	
	mper.end_analysis();
	
	BRAIN_CK((f_nmp == NULL_PT) || ! f_nmp->is_active() || f_nmp->na_is_head);
	if((f_nmp != NULL_PT) && ! f_nmp->is_active()){
		f_nmp->na_is_head = true;
		f_nmp->map_add_to_release();
	}
	if(f_nmp != NULL_PT){ 
		f_nmp->map_reset_all_notes_and_tags(); 
	}
	BRAIN_CK(br_ne_tot_tag1 == 0);
	
	write_all_neuromaps(to_wrt); // deactivates final wrt maps
	
	BRAIN_CK(ck_cov_flags());
	
	release_all_neuromaps();
	
	BRAIN_DBG(bool do_finds = true);
	DBG_COMMAND(4, do_finds = false);
	BRAIN_CK_PRT((do_finds || fst_dct.equal_to_dct(out_dct)),
			os << "________\n";
			os << "fst_dct=" << fst_dct;
			os << "out_dct=" << out_dct;
	);
	
	DBG_PRT(40, os << "AFT_ana=" << bj_eol; print_trail(os);
		os << out_dct << bj_eol
	);
}

bool
deduction::equal_to_dct(deduction& dct2){
	bool c1 = dt_motives.equal_to(dct2.dt_motives);
	bool c2 = (dt_forced == dct2.dt_forced);
	bool c3 = (dt_target_level == dct2.dt_target_level);
	bool all_cc = (c1 && c2 && c3);
	return all_cc;
}

void
brain::release_all_neuromaps(){
	row<neuromap*>& all_nmps = br_nmps_to_release;
	
	while(! all_nmps.is_empty()){
		neuromap* nmp_pt = all_nmps.pop();
		if(nmp_pt == NULL_PT){
			continue;
		}
		neuromap& nmp = *nmp_pt;
		BRAIN_CK_PRT(nmp.na_is_head, os << "____" << bj_eol << "NMP=" << &nmp);
		nmp.full_release();
	}
}

bool
prop_signal::is_ps_of_qua(quanton& qua, neuromap* dbg_nmp){
	if(ps_quanton == NULL_PT){ 
		DBG_PRT_WITH(55, qua, os << "NULL ps_qua !!!!");
		return false; 
	}
	BRAIN_CK(get_dbg_slv() != NULL_PT);
	if(ps_quanton != &qua){ 
		DBG_PRT(55, os << "NOT_to_wrt (bad_qua_1 !!) ps=" << this 
			<< " qu=" << &qua << " nmp=" << (void*)dbg_nmp);
		return false; 
	}
	DBG_PRT(43, os << "D." << &qua 
		<< " pti=" << ps_tier
		<< " qti=" << qua.qu_tier
	);
	if(ps_tier != qua.qu_tier){ 
		DBG_PRT(55, os << "NOT_to_wrt (bad_qua_ 2!!) ps=" << this 
			<< " qu=" << &qua << " nmp=" << (void*)dbg_nmp);
		return false; 
	}
	return true;
}

neuromap*
quanton::get_nmp_to_write(brain& brn){
	quanton& qua = *this;
	
	BRAIN_CK(is_pos());
	BRAIN_CK(in_qu_dominated(brn));
	
	if(! is_pos()){ 
		DBG_PRT(55, os << "NOT_to_wrt. NOT pos !!!");
		return NULL_PT; 
	}
	if(! in_qu_dominated(brn)){ 
		DBG_PRT(55, os << "NOT_to_wrt. NOT dom !!!");
		return NULL_PT; 
	}
	quanton& opp = opposite();
	if(opp.is_mono()){
		DBG_PRT(55, os << "NOT_to_wrt. Mono=" << opp);
		return NULL_PT; 
	}
	
	neuromap* to_wrt = qu_curr_nemap;
	
	if(to_wrt == NULL_PT){ 
		DBG_PRT(55, os << "NOT_to_wrt. NULL curr_nmp. NULL to_wrt");
		return NULL_PT; 
	}

	BRAIN_CK(to_wrt->is_active());
	if(! to_wrt->is_active()){ 
		DBG_PRT(55, os << "NOT_to_wrt (not_active) to_wrt=(" << (void*)to_wrt << ")");
		return NULL_PT; 
	}
	
	prop_signal o_ps;
	o_ps = to_wrt->na_next_psig;
	/*if(is_choice()){
		o_ps.init_prop_signal(to_wrt->na_orig_cho, NULL_PT, to_wrt->na_orig_ti);
	}*/
	
	//if(! to_wrt->na_next_psig.is_ps_of_qua(qua, to_wrt)){ 
	if(! o_ps.is_ps_of_qua(qua, to_wrt)){
		DBG_PRT(55, os << "NOT_to_wrt (not_is_ps)\n to_wrt=" << to_wrt << "\n";
			os << " o_ps=" << o_ps << "\n";
			os << " qua=" << &qua << "\n";
		);
		return NULL_PT; 
	}
	
	return to_wrt;
}

void
prop_signal::make_ps_dominated(brain& brn, row<neuromap*>& to_wrt){
	if(ps_quanton != NULL_PT){
		ps_quanton->make_qu_dominated(brn);
		if(ps_quanton->is_qu_end_of_nmp()){
			DBG_PRT(43, os << "mk_ps_dom(end)=" << this);
			neuromap* nmp = ps_quanton->get_nmp_to_write(brn);
			//neuromap* nmp = ps_quanton->qu_curr_nemap;
			if(nmp != NULL_PT){
				DBG_PRT(55, os << "ADDING_to_wrt ";
					brn.print_trail(os);
					os << "ps=" << this << " nmp=" << nmp
				);
				to_wrt.push(nmp);
			}
		}
	}
	if(ps_source != NULL_PT){
		ps_source->make_ne_dominated(brn);
	}
}

bool
neuromap::has_qua_tier(quanton& qua){
	long min_ti = get_min_ti();
	long max_ti = get_max_ti();
	long qti = qua.qu_tier;
	bool in_sec = ((qti >= min_ti) && (qti <= max_ti));
	return in_sec;
}

/*
bool
neuromap::is_last_forced(quanton& qua){
	row<prop_signal>& all_fo = na_forced;
	if(all_fo.is_empty()){
		return false;
	}
	prop_signal& lst_ps = all_fo.last();
	return lst_ps.is_ps_of_qua(qua);
}*/

neuromap*
cut_neuromap(neuromap* nmp, neuromap* nxt_nmp){
	BRAIN_CK(nmp != NULL_PT);
	BRAIN_CK(nxt_nmp != NULL_PT);
	BRAIN_CK_PRT((nmp == nxt_nmp) || (find_parent_of(*nmp, *nxt_nmp) != NULL_PT), 
		os << "________\n ABORT_DATA\n";
		os << " nmp=" << nmp << "\n";
		os << " nxt_nmp=" << nxt_nmp << "\n";
		nmp->print_all_subnmp(os, true);
	);
	
	neuromap* old_nmp = nmp;
	
	nmp = nxt_nmp->na_submap;
	nxt_nmp->na_submap = NULL_PT;
	
	old_nmp->na_is_head = true;
	
	old_nmp->map_reset_all_notes_and_tags();
	old_nmp->full_release();
	
	nmp->na_is_head = true;
	return nmp;
}

bool
neuromap::map_is_setup_nmp(long nxt_lv, long st_lv){
	if(! map_can_activate()){
		return false;
	}
	if(na_orig_lv <= nxt_lv){
		return true;
	}
	if(na_orig_lv == st_lv){
		return true;
	}
	return false;
}

neuromap*
analyser::calc_setup_neuromap(neuromap* nmp, long nxt_lv, long ded_lv){
	BRAIN_CK(nmp != NULL_PT);
	if(nmp->map_is_setup_nmp(nxt_lv, ded_lv)){
		return nmp;
	}
	
	neuromap* nxt_nmp = nmp;
	while(nxt_nmp != NULL_PT){
		neuromap* sub_nmp = nxt_nmp->na_submap;
		if((sub_nmp != NULL_PT) && sub_nmp->map_is_setup_nmp(nxt_lv, ded_lv)){
			nmp = cut_neuromap(nmp, nxt_nmp);
			BRAIN_CK(nmp == sub_nmp);
			break;
		}
		nxt_nmp = nxt_nmp->na_submap;
	}
	
	if((nmp != NULL_PT) && ! nmp->map_is_setup_nmp(nxt_lv, ded_lv)){
		nmp->map_reset_all_notes_and_tags();
		nmp->full_release();
		nmp = NULL_PT;
	}
	BRAIN_CK_PRT(((nmp == NULL_PT) || nmp->map_can_activate()), 
		os << "__________\n ABORT_DATA \n";
		os << " nmp=" << nmp;
	);
	return nmp;
}

/*
neuromap*
analyser::calc_orig_neuromap(neuromap* nmp){
	if(! found_learned()){
		return nmp;
	}
	BRAIN_CK(found_learned());
	quanton& qua = first_learned();
	
	DBG_PRT(54, os << "in_nmp=" << nmp << "\n";
		os << "qua=" << &qua << "\n";
		get_de_brain().print_trail(os);
	);

	neuromap* nxt_nmp = nmp;
	while(nxt_nmp != NULL_PT){
		DBG_PRT(54, os << "nxt_nmp=" << nxt_nmp << "\n");
		if(nxt_nmp->has_qua_tier(qua)){
			nmp = cut_neuromap(nmp, nxt_nmp);
			break;
		}
		nxt_nmp = nxt_nmp->na_submap;
	}
	BRAIN_CK((nmp == NULL_PT) || nmp->map_ck_orig());
	return nmp;
}*/

void
neuromap::map_reset_all_notes_and_tags(){
	brain& brn = get_brn();
	row_neuron_t& all_neus = brn.br_tmp_ck_neus;
	all_neus.clear();
	map_get_all_neus(all_neus);
	reset_all_tag1(brn, all_neus);
	
	
	map_forced_reset_all_note2_n_tag2();
	map_propag_reset_all_note3_n_tag3();
}

void
analyser::make_noted_dominated_and_deduced(row<neuromap*>& to_wrt){
	brain& brn = get_de_brain();
	row<prop_signal>& all_noted = de_all_noted;
	
	DBG_PRT(43, os << "mk dom_noted=\n" << this);
	
	make_all_ps_dominated(brn, all_noted, to_wrt);
	
	BRAIN_DBG(qlayers_ref& qlr = de_ref);
	BRAIN_CK(last_qu_noted() == qlr.get_curr_quanton());
}

neuromap*
analyser::neuromap_setup_analysis(long nxt_lv, neuromap* in_nmp, deduction& dct)
{
	brain& brn = get_de_brain();
	if(brn.level() == ROOT_LEVEL){
		BRAIN_CK(nxt_lv == INVALID_LEVEL);
		return NULL_PT;
	}
	
	BRAIN_CK(nxt_lv != INVALID_LEVEL);
	BRAIN_CK((in_nmp == NULL_PT) || (in_nmp->na_orig_lv > nxt_lv));
	BRAIN_CK((in_nmp == NULL_PT) || ! in_nmp->na_is_head);
	
	if(in_nmp != NULL_PT){
		neuromap* no_mono_nmp = in_nmp->na_nxt_no_mono;
		BRAIN_CK(no_mono_nmp != NULL_PT);
		DBG_PRT(65, 
			ch_string msg = "SETUP mk_dom:";
			no_mono_nmp->map_dbg_update_html_file(msg);
		);
		DBG_PRT(65, os << "SETUP mk_dom\n";
			os << " in_nmp=" << in_nmp << "\n";
			os << " nx_mon=" << no_mono_nmp << "\n";
			no_mono_nmp->print_all_subnmp(os, true);
		);
		
		no_mono_nmp->map_make_all_dominated();
	}
	neuromap* orig_nmp = calc_neuromap(ROOT_LEVEL, in_nmp, false, true);
	
	long d_lv = dct.get_deduc_lv();
	neuromap* setup_nmp = calc_setup_neuromap(orig_nmp, nxt_lv, d_lv);
	if(setup_nmp != NULL_PT){
		DBG_PRT(65, os << "SETUP NMP=" << setup_nmp << "\n";
			setup_nmp->print_all_subnmp(os, true);
		);
		
		BRAIN_CK(setup_nmp->map_can_activate());
		BRAIN_CK(setup_nmp->map_is_setup_nmp(nxt_lv, d_lv));
		BRAIN_CK(setup_nmp->map_ck_all_qu_dominated(dbg_call_1));
		BRAIN_CK(setup_nmp->map_ck_all_ne_dominated(dbg_call_1));
		
		setup_nmp->na_is_head = true;
	}
	
	DBG_PRT_COND(66, (setup_nmp != NULL_PT),
		os << HTMi_h1 << "TOP_SETUP" << HTMe_h1 << "nmp=" << setup_nmp << "\n";
		ch_string msg = "top_setup";
		os << "\n";
		brn.print_trail(os);
		os << "\n";
		setup_nmp->map_dbg_update_html_file(msg);
	);
	
	neuromap* nxt_nmp = setup_nmp;
	while(nxt_nmp != NULL_PT){
		nxt_nmp->map_cond_activate(dbg_call_1);
		nxt_nmp = nxt_nmp->na_submap;
	}
	BRAIN_CK((setup_nmp == NULL_PT) || setup_nmp->is_active());
	return setup_nmp;
}

bool
ck_all_active(row<neuromap*>& all_act){
#ifdef FULL_DEBUG
	for(long aa = 0; aa < all_act.size(); aa++){
		BRAIN_CK(all_act[aa] != NULL_PT);
		BRAIN_CK(all_act[aa]->is_active());
	}
#endif
	return true;
}

bool
neuromap::ck_act_idx(){
#ifdef FULL_DEBUG
	if(na_active_idx == INVALID_IDX){
		return true;
	}
	brain* pt_brn = get_dbg_brn();
	BRAIN_CK(pt_brn != NULL_PT);
	brain& brn = *pt_brn;
	
	BRAIN_CK(brn.br_maps_active.is_valid_idx(na_active_idx));
	BRAIN_CK(brn.br_maps_active[na_active_idx] == this);
#endif
	return true;
}

bool
leveldat::ck_maps_active(){
#ifdef FULL_DEBUG
	brain* pt_brn = get_dbg_brn();
	BRAIN_CK(pt_brn != NULL_PT);
	brain& brn = *pt_brn;
	
	BRAIN_CK(ck_all_active(brn.br_maps_active));
	
	row<neuromap*>& all_act = brn.br_dbg_all_act;
	
	all_act.clear();
	ld_nmps_to_write.append_all_as<neuromap>(all_act, true);
	BRAIN_CK(ck_all_active(all_act));
	
	all_act.clear();
	ld_nmp_setup.append_all_as<neuromap>(all_act, true);
	BRAIN_CK(ck_all_active(all_act));
#endif
	return true;
}

void
leveldat::let_maps_go(brain& brn){
	BRAIN_CK(ck_maps_active());
	while(! ld_nmps_to_write.is_alone()){
		brn.deactivate_last_map_to_wrt();
	}
	while(! ld_nmp_setup.is_alone()){
		brn.deactivate_last_setup_map();
	}
}

neuromap*
analyser::neuromap_find_analysis(analyser& deducer, 
					long& nxt_lv, deduction& nxt_dct, row<neuromap*>& to_wrt)
{
	brain& brn = get_de_brain();
	row_quanton_t& nmp_causes = brn.br_tmp_f_analysis;
	if(brn.level() == ROOT_LEVEL){
		DBG_PRT(39, os << "find.NULL_NMP root level");
		BRAIN_CK(nxt_dct.dt_target_level == INVALID_LEVEL);
		return NULL_PT;
	}
	
	if(! to_wrt.is_empty() && brn.lv_has_setup_nmp(nxt_lv + 1)){
		DBG_PRT(39, os << "find.NULL_NMP !to_wrt.empty && h_s_lv+1");
		return NULL_PT;
	}
	
	nxt_dct.init_deduction();
	
	BRAIN_CK(brn.br_qu_tot_note0 == 0);
	
	neuromap* out_nmp = calc_neuromap(nxt_lv, NULL_PT, true, false);
	
	BRAIN_CK(out_nmp != NULL_PT);
	BRAIN_CK(out_nmp->na_orig_lv == (nxt_lv + 1));
	
	neuromap* last_found = NULL_PT;
	BRAIN_DBG(long num_fnd = 0);
	while(out_nmp != NULL_PT){
		BRAIN_DBG(num_fnd++);
		
		neuromap* to_find = out_nmp;
		if(to_find->is_na_mono()){
			to_find = to_find->na_nxt_no_mono;
			BRAIN_CK(to_find != NULL_PT);
		}
		if((to_find != last_found) && ! to_find->map_find()){
			DBG_PRT(39, os << "CANNOT find nmp=" << (void*)(out_nmp) << "\n";
				os << " nxt_dct=" << nxt_dct << "\n";
				os << " nxt_lv=" << nxt_lv << "\n";
				to_wrt.print_row_data(os, true, "\n");
			);
			DBG_PRT(150, 
				ch_string msg = HTMi_h1 "FindNOT" HTMe_h1;
				to_find->map_dbg_update_html_file(msg);
			);
			break;
		}
		if(! out_nmp->is_na_mono()){
			last_found = out_nmp;
		}
		
		BRAIN_DBG(
			long& max_ns = brn.br_dbg.dbg_max_fnd_num_subnmp;
			if(out_nmp->na_dbg_num_submap > max_ns){
				max_ns = out_nmp->na_dbg_num_submap;
			}
		);

		BRAIN_CK(brn.br_qu_tot_note0 == 0);
		
		DBG_PRT(39, os << "found nmp=" << (void*)(out_nmp));
		if(nxt_lv <= 0){
			DBG_PRT(39, os << "find stop. (nxt_lv <= 0). nmp=" << (void*)(out_nmp));
			break;
		}
		
		nmp_causes.clear();
		out_nmp->map_get_all_upper_quas(nmp_causes);
	
		deduction tmp_dct;
		tmp_dct.init_deduction();
		deducer.deduction_analysis(nmp_causes, tmp_dct);
 
		long old_lv = nxt_lv;
		if(tmp_dct.dt_target_level >= old_lv){
			BRAIN_CK(false);
			DBG_PRT(39, os << "find stop. (tgt_lv >= old_lv). nmp=" << (void*)(out_nmp));
			break;
		}
		
		deducer.make_noted_dominated_and_deduced(to_wrt);
		//brn.get_all_retracted_to_write(tmp_dct, to_wrt);
		
		nxt_dct.init_deduction();
		tmp_dct.move_to_dct(nxt_dct);
		nxt_lv = nxt_dct.dt_target_level;
		
		BRAIN_CK(nxt_lv < out_nmp->na_orig_lv);
		BRAIN_CK_PRT(nxt_lv < old_lv, os << "________________CK_FAILED______\n";
			brn.dbg_prt_margin(os);
			os << *out_nmp << "\n";
			os << "nmp_causes=" << nmp_causes << "\n";
			os << " na_lv=" << out_nmp->na_orig_lv;
			os << " nxt_lv=" << nxt_lv;
			os << " old_lv=" << old_lv << "\n";
			os << " curr_qlv=" << de_ref.get_curr_qlevel();
			os << ":::::::::::::::::::::::::::::::::::::::::::::\n"; 
			brn.print_trail(os);
		);
		BRAIN_REL_CK(nxt_lv < old_lv);

		if(! to_wrt.is_empty() && brn.lv_has_setup_nmp(nxt_lv + 1)){
			DBG_PRT(39, os << "find stop. has_to_wrt && has_stup. nmp=" << (void*)(out_nmp));
			break;
		}
		if(deducer.found_learned()){
			DBG_PRT(39, os << "find stop. found_lrnd. nmp=" << (void*)(out_nmp));
			break;
		}

		neuromap* in_nmp = out_nmp;
		out_nmp = calc_neuromap(nxt_lv, in_nmp, true, false);
		BRAIN_CK(out_nmp != NULL_PT);
		BRAIN_CK(out_nmp != in_nmp);
	}
	return out_nmp;
}

cy_quk_t
quanton::get_cy_kind(){
	cy_quk_t kk = cq_reg;
	bool is_cho = is_choice();
	bool is_lrn = is_learned_choice();
	//bool is_mon = opposite().is_mono();
	if(is_cho){ kk = cq_cho; }
	if(is_lrn){ kk = cq_for; }
	BRAIN_CK_PRT((qlevel() == ROOT_LEVEL) || is_cho || is_lrn, 
		os << "____________\n"; 
		os << " qua=" << this;
	);
	//if(is_mon){ kk = cq_mono; }
	return kk;
}

/*
void
brain::get_all_retracted_to_write(deduction& dct, row<neuromap*>& to_wrt){
	//brain& brn = get_de_brain();
	
	BRAIN_CK(dct.dt_forced != NULL_PT);
	long cur_lv = dct.dt_forced->qlevel();
	long nxt_lv = dct.dt_target_level;
	BRAIN_CK(nxt_lv < cur_lv);
	
	if(nxt_lv == ROOT_LEVEL){
		return;
	}
	
	cur_lv--;
	while(nxt_lv < cur_lv){
		leveldat& lv_dat = get_data_level(cur_lv);
		//lv_dat.ld_nmps_to_write.append_all_as<neuromap>(to_wrt);
		lv_dat.ld_nmp_setup.append_all_as<neuromap>(to_wrt);
		
		cur_lv--;
	}
}
*/

