/*****************************************************************************\
 *  node_info.c - Functions related to node display 
 *  mode of sview.
 *****************************************************************************
 *  Copyright (C) 2004-2006 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Danny Auble <da@llnl.gov>
 *
 *  UCRL-CODE-217948.
 *  
 *  This file is part of SLURM, a resource management program.
 *  For details, see <http://www.llnl.gov/linux/slurm/>.
 *  
 *  SLURM is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *  
 *  SLURM is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with SLURM; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#include "src/sview/sview.h"

#define _DEBUG 0
DEF_TIMERS;

enum { 
	SORTID_POS = POS_LOC,
	SORTID_NAME, 
	SORTID_STATE,
	SORTID_STATE_NUM,
	SORTID_CPUS, 
	SORTID_MEMORY, 
	SORTID_DISK, 
	SORTID_WEIGHT, 
	SORTID_FEATURES, 
	SORTID_REASON,
	SORTID_UPDATED, 
	SORTID_CNT
};

static display_data_t display_data_node[] = {
	{G_TYPE_INT, SORTID_POS, NULL, FALSE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_STRING, SORTID_NAME, "Name", TRUE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_STRING, SORTID_STATE, "State", TRUE, 0, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_INT, SORTID_STATE_NUM, NULL, FALSE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_INT, SORTID_CPUS, "CPU Count", TRUE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_STRING, SORTID_MEMORY, "Real Memory", TRUE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_STRING, SORTID_DISK, "Tmp Disk", TRUE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_INT, SORTID_WEIGHT,"Weight", FALSE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_STRING, SORTID_FEATURES, "Features", FALSE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_STRING, SORTID_REASON, "Reason", FALSE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_INT, SORTID_UPDATED, NULL, FALSE, -1, refresh_node,
	 create_model_node, admin_edit_node},
	{G_TYPE_NONE, -1, NULL, FALSE, -1}
};

static display_data_t options_data_node[] = {
	{G_TYPE_INT, SORTID_POS, NULL, FALSE, -1},
	{G_TYPE_STRING, INFO_PAGE, "Full Info", TRUE, NODE_PAGE},
	{G_TYPE_STRING, JOB_PAGE, "Jobs", TRUE, NODE_PAGE},
#ifdef HAVE_BG
	{G_TYPE_STRING, BLOCK_PAGE, "Blocks", TRUE, NODE_PAGE},
#endif
	{G_TYPE_STRING, PART_PAGE, "Partition", TRUE, NODE_PAGE},
	{G_TYPE_STRING, SUBMIT_PAGE, "Job Submit", TRUE, NODE_PAGE},
	{G_TYPE_NONE, -1, NULL, FALSE, -1}
};

static display_data_t *local_display_data = NULL;

static void _layout_node_record(GtkTreeView *treeview,
				node_info_t *node_ptr,
				int update)
{
	char tmp_cnt[50];
	char *upper = NULL, *lower = NULL;		     
	GtkTreeIter iter;
	GtkTreeStore *treestore = 
		GTK_TREE_STORE(gtk_tree_view_get_model(treeview));
	if(!treestore)
		return;
	
	add_display_treestore_line(update, treestore, &iter,
				   display_data_node[SORTID_NAME].name,
				   node_ptr->name);
				   
	upper = node_state_string(node_ptr->node_state);
	lower = str_tolower(upper);
	add_display_treestore_line(update, treestore, &iter,
				   display_data_node[SORTID_STATE].name,
				   lower);
	xfree(lower);
	
	convert_num_unit((float)node_ptr->cpus, tmp_cnt, UNIT_MEGA);
	add_display_treestore_line(update, treestore, &iter, 
				   display_data_node[SORTID_CPUS].name,
				   tmp_cnt);

	convert_num_unit((float)node_ptr->real_memory, tmp_cnt, UNIT_MEGA);
	add_display_treestore_line(update, treestore, &iter, 
				   display_data_node[SORTID_MEMORY].name,
				   tmp_cnt);

	convert_num_unit((float)node_ptr->tmp_disk, tmp_cnt, UNIT_MEGA);
	add_display_treestore_line(update, treestore, &iter, 
				   display_data_node[SORTID_DISK].name,
				   tmp_cnt);
	snprintf(tmp_cnt, sizeof(tmp_cnt), "%u", node_ptr->weight);
	add_display_treestore_line(update, treestore, &iter,
				   display_data_node[SORTID_WEIGHT].name, 
				   tmp_cnt);
	add_display_treestore_line(update, treestore, &iter, 
				   display_data_node[SORTID_FEATURES].name, 
				   node_ptr->features);
	add_display_treestore_line(update, treestore, &iter,
				   display_data_node[SORTID_REASON].name, 
				   node_ptr->reason);
	return;
}

static void _update_node_record(node_info_t *node_ptr,
				GtkTreeStore *treestore, GtkTreeIter *iter)
{
	char tmp_cnt[7];
	char *upper = NULL, *lower = NULL;		     
	
	gtk_tree_store_set(treestore, iter, SORTID_NAME, node_ptr->name, -1);

	upper = node_state_string(node_ptr->node_state);
	lower = str_tolower(upper);
	gtk_tree_store_set(treestore, iter, SORTID_STATE, lower, -1);
	xfree(lower);

	gtk_tree_store_set(treestore, iter, SORTID_STATE_NUM, 
			   node_ptr->node_state, -1);
	gtk_tree_store_set(treestore, iter, SORTID_CPUS, node_ptr->cpus, -1);

	convert_num_unit((float)node_ptr->real_memory, tmp_cnt, UNIT_MEGA);
	gtk_tree_store_set(treestore, iter, SORTID_MEMORY, tmp_cnt, -1);
	convert_num_unit((float)node_ptr->tmp_disk, tmp_cnt, UNIT_MEGA);
	gtk_tree_store_set(treestore, iter, SORTID_DISK, tmp_cnt, -1);
	gtk_tree_store_set(treestore, iter, SORTID_WEIGHT, 
			   node_ptr->weight, -1);
	gtk_tree_store_set(treestore, iter, SORTID_FEATURES, 
			   node_ptr->features, -1);
	gtk_tree_store_set(treestore, iter, SORTID_REASON, 
			   node_ptr->reason, -1);
	gtk_tree_store_set(treestore, iter, SORTID_UPDATED, 1, -1);	
	
	return;
}

static void _append_node_record(node_info_t *node_ptr,
				GtkTreeStore *treestore, GtkTreeIter *iter,
				int line)
{
	gtk_tree_store_append(treestore, iter, NULL);
	gtk_tree_store_set(treestore, iter, SORTID_POS, line, -1);
	_update_node_record(node_ptr, treestore, iter);
}

static void _update_info_node(List info_list, GtkTreeView *tree_view)
{
	GtkTreePath *path = gtk_tree_path_new_first();
	GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
	GtkTreeIter iter;
	node_info_t *node_ptr = NULL;
	int line = 0;
	char *name;
	ListIterator itr = NULL;
	sview_node_info_t *sview_node_info = NULL;

	/* get the iter, or find out the list is empty goto add */
	if (gtk_tree_model_get_iter(model, &iter, path)) {
		/* make sure all the partitions are still here */
		while(1) {
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
					   SORTID_UPDATED, 0, -1);	
			if(!gtk_tree_model_iter_next(model, &iter)) {
				break;
			}
		}
	}
	itr = list_iterator_create(info_list);
	while ((sview_node_info = (sview_node_info_t*) list_next(itr))) {
		node_ptr = sview_node_info->node_ptr;
		/* get the iter, or find out the list is empty goto add */
		if (!gtk_tree_model_get_iter(model, &iter, path)) {
			goto adding;
		}
		while(1) {
			/* search for the node name and check to see if 
			   it is in the list */
			gtk_tree_model_get(model, &iter, SORTID_NAME, 
					   &name, -1);
			if(!strcmp(name, node_ptr->name)) {
				/* update with new info */
				g_free(name);
				_update_node_record(node_ptr, 
						    GTK_TREE_STORE(model), 
						    &iter);
				goto found;
			}
			g_free(name);
			/* see what line we were on to add the next one 
			   to the list */
			gtk_tree_model_get(model, &iter, SORTID_POS, 
					   &line, -1);
			if(!gtk_tree_model_iter_next(model, &iter)) {
				line++;
				break;
			}
		}
	adding:
		_append_node_record(node_ptr, GTK_TREE_STORE(model), &iter,
				    line);
	found:
		;
	}
	list_iterator_destroy(itr);
	
       	gtk_tree_path_free(path);
	/* remove all old nodes */
	remove_old(model, SORTID_UPDATED);
}

static void _node_info_list_del(void *object)
{
	sview_node_info_t *sview_node_info = (sview_node_info_t *)object;

	if (sview_node_info) {
		xfree(sview_node_info);
	}
}

void _display_info_node(List info_list,	popup_info_t *popup_win)
{
	specific_info_t *spec_info = popup_win->spec_info;
	char *name = (char *)spec_info->data;
	int found = 0;
	node_info_t *node_ptr = NULL;
	GtkTreeView *treeview = NULL;
	int update = 0;
	ListIterator itr = NULL;
	sview_node_info_t *sview_node_info = NULL;
	int i = -1;

	if(!spec_info->data) {
		goto finished;
	}
need_refresh:
	if(!spec_info->display_widget) {
		treeview = create_treeview_2cols_attach_to_table(
			popup_win->table);
		spec_info->display_widget = 
			gtk_widget_ref(GTK_WIDGET(treeview));
	} else {
		treeview = GTK_TREE_VIEW(spec_info->display_widget);
		update = 1;
	}
	
	itr = list_iterator_create(info_list);
	while ((sview_node_info = (sview_node_info_t*) list_next(itr))) {
		node_ptr = sview_node_info->node_ptr;
		i++;
		if(!strcmp(node_ptr->name, name)) {
#ifdef HAVE_BG
			/* get other blocks that are on this bp */
			add_extra_bluegene_buttons(
				&popup_win->grid_button_list,
				i, &i);
			i--;
#else
			/* this may need to go away in the future
			   after add_extra_cr_buttons is functional */
			get_button_list_from_main(&popup_win->grid_button_list,
						  i, i, i);
			/* drill into node putting buttons for each
			   core and what not */
			add_extra_cr_buttons(&popup_win->grid_button_list,
					     node_ptr);
#endif
			_layout_node_record(treeview, node_ptr, update);
			found = 1;
			break;
		}		
	}
	list_iterator_destroy(itr);
	if(!found) {
		if(!popup_win->not_found) { 
#ifdef HAVE_BG
			char *temp = "BP NOT FOUND\n";
#else
			char *temp = "NODE NOT FOUND\n";
#endif
			GtkTreeIter iter;
			GtkTreeModel *model = NULL;
			
			/* only time this will be run so no update */
			model = gtk_tree_view_get_model(treeview);
			add_display_treestore_line(0, 
						   GTK_TREE_STORE(model), 
						   &iter,
						   temp, "");
		}
		popup_win->not_found = true;
	} else {
		if(popup_win->not_found) { 
			popup_win->not_found = false;
			gtk_widget_destroy(spec_info->display_widget);
			
			goto need_refresh;
		}
		put_buttons_in_table(popup_win->grid_table,
				     popup_win->grid_button_list);
	}
	gtk_widget_show(spec_info->display_widget);

finished:
	return;
}

extern void refresh_node(GtkAction *action, gpointer user_data)
{
	popup_info_t *popup_win = (popup_info_t *)user_data;
	xassert(popup_win != NULL);
	xassert(popup_win->spec_info != NULL);
	xassert(popup_win->spec_info->title != NULL);
	popup_win->force_refresh = 1;
	specific_info_node(popup_win);
}

/* don't destroy the list from this function */
extern List create_node_info_list(node_info_msg_t *node_info_ptr, int changed)
{
	static List info_list = NULL;
	int i = 0;
	sview_node_info_t *sview_node_info_ptr = NULL;
	node_info_t *node_ptr = NULL;
	
	if(!changed && info_list) {
		goto update_color;
	}
	
	if(info_list) {
		list_destroy(info_list);
	}

	info_list = list_create(_node_info_list_del);
	if (!info_list) {
		g_print("malloc error\n");
		return NULL;
	}
	
	for (i=0; i<node_info_ptr->record_count; i++) {
		node_ptr = &(node_info_ptr->node_array[i]);
		if (!node_ptr->name || (node_ptr->name[0] == '\0'))
			continue;	/* bad node */
	
		sview_node_info_ptr = xmalloc(sizeof(sview_node_info_t));
		list_append(info_list, sview_node_info_ptr);
		sview_node_info_ptr->node_ptr = node_ptr;		
	}
update_color:
	
	return info_list;
}

extern int get_new_info_node(node_info_msg_t **info_ptr, int force)
{
	static node_info_msg_t *node_info_ptr = NULL, *new_node_ptr = NULL;
	uint16_t show_flags = 0;
	int error_code = SLURM_SUCCESS;
	time_t now = time(NULL);
	static time_t last;
		
	if(!force && ((now - last) < global_sleep_time)) {
		*info_ptr = node_info_ptr;
		return error_code;
	}
	last = now;

	show_flags |= SHOW_ALL;
	if (node_info_ptr) {
		error_code = slurm_load_node(node_info_ptr->last_update,
					     &new_node_ptr, show_flags);
		if (error_code == SLURM_SUCCESS)
			slurm_free_node_info_msg(node_info_ptr);
		else if (slurm_get_errno() == SLURM_NO_CHANGE_IN_DATA) {
			error_code = SLURM_NO_CHANGE_IN_DATA;
			new_node_ptr = node_info_ptr;
		}
	} else
		error_code = slurm_load_node((time_t) NULL, &new_node_ptr, 
					     show_flags);
	node_info_ptr = new_node_ptr;
	*info_ptr = new_node_ptr;
	return error_code;
}

extern int update_state_node(GtkTreeStore *treestore, GtkTreeIter *iter, 
			     int text_column, int num_column,
			     const char *new_text,
			     update_node_msg_t *node_msg)
{
	uint16_t state = (uint16_t) NO_VAL;
	char *upper = NULL, *lower = NULL;		     
	int i = 0;
	int rc = SLURM_SUCCESS;

	node_msg->reason = NULL;

	if(!strcmp("drain", new_text)) {
		state = NODE_STATE_DRAIN;
		node_msg->reason = get_reason();
	} else if(!strcmp("resume", new_text)) {
		state = NODE_RESUME;
	} else {
		for(i = 0; i < NODE_STATE_END; i++) {
			upper = node_state_string(i);
			lower = str_tolower(upper);
			if(!strcmp(lower, new_text)) {
				state = i;
				xfree(lower);
				break;
			}
			xfree(lower);
		}
	}
	node_msg->node_state = (uint16_t)state;
	
	if(slurm_update_node(node_msg) == SLURM_SUCCESS) {
	       	gtk_tree_store_set(treestore, iter, text_column, new_text,
				   num_column,	state, -1);
		upper = g_strdup_printf("Node(s) %s state changed to %s",
					node_msg->node_names,
					new_text);
		display_edit_note(upper);
		g_free(upper);
	}
	xfree(node_msg->reason);
	return rc;
}
			
extern GtkListStore *create_model_node(int type)
{
	GtkListStore *model = NULL;
	GtkTreeIter iter;
	char *upper = NULL, *lower = NULL;		     
	int i=0;
	
	switch(type) {
		case SORTID_STATE:
		model = gtk_list_store_new(2, G_TYPE_STRING,
					   G_TYPE_INT);
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter,
				   0, "drain",
				   1, i,
				   -1);	
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter,
				   0, "NoResp",
				   1, i,
				   -1);	
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter,
				   0, "resume",
				   1, i,
				   -1);	
		for(i = 0; i < NODE_STATE_END; i++) {
			upper = node_state_string(i);
			gtk_list_store_append(model, &iter);
			lower = str_tolower(upper);
			gtk_list_store_set(model, &iter,
					   0, lower,
					   1, i,
					   -1);
			xfree(lower);
		}
						
		break;

	}
	return model;
}

extern void admin_edit_node(GtkCellRendererText *cell,
			    const char *path_string,
			    const char *new_text,
			    gpointer data)
{
	GtkTreeStore *treestore = GTK_TREE_STORE(data);
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	update_node_msg_t node_msg;
	
	int column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), 
						       "column"));
	gtk_tree_model_get_iter(GTK_TREE_MODEL(treestore), &iter, path);
	switch(column) {
	case SORTID_STATE:
		gtk_tree_model_get(GTK_TREE_MODEL(treestore), &iter, 
				   SORTID_NAME, 
				   &node_msg.node_names, -1);
		update_state_node(treestore, &iter, 
				  SORTID_STATE, SORTID_STATE_NUM,
				  new_text, &node_msg);
		g_free(node_msg.node_names);
	}
	gtk_tree_path_free (path);
	
	g_static_mutex_unlock(&sview_mutex);
}

extern void get_info_node(GtkTable *table, display_data_t *display_data)
{
	int error_code = SLURM_SUCCESS;
	static int view = -1;
	node_info_msg_t *node_info_ptr = NULL;
	char error_char[100];
	GtkWidget *label = NULL;
	GtkTreeView *tree_view = NULL;
	static GtkWidget *display_widget = NULL;
	List info_list = NULL;
	int changed = 1;
	int i = 0;
	sview_node_info_t *sview_node_info_ptr = NULL;
	ListIterator itr = NULL;
	
	if(display_data)
		local_display_data = display_data;
	if(!table) {
		display_data_node->set_menu = local_display_data->set_menu;
		return;
	}
	if(display_widget && toggled) {
		gtk_widget_destroy(display_widget);
		display_widget = NULL;
		goto display_it;
	}

	if((error_code = get_new_info_node(&node_info_ptr, force_refresh))
	   == SLURM_NO_CHANGE_IN_DATA) { 
		if(!display_widget || view == ERROR_VIEW)
			goto display_it;
		changed = 0;
		goto display_it;
	} 

	if (error_code != SLURM_SUCCESS) {
		if(view == ERROR_VIEW)
			goto end_it;
		view = ERROR_VIEW;
		if(display_widget)
			gtk_widget_destroy(display_widget);
		sprintf(error_char, "slurm_load_node: %s",
			slurm_strerror(slurm_get_errno()));
		label = gtk_label_new(error_char);
		gtk_table_attach_defaults(table, 
					  label,
					  0, 1, 0, 1); 
		gtk_widget_show(label);	
		display_widget = gtk_widget_ref(label);
		goto end_it;
	}
display_it:
	info_list = create_node_info_list(node_info_ptr, changed);

	if(!info_list)
		return;
	i=0;
	/* set up the grid */
	itr = list_iterator_create(info_list);
	while ((sview_node_info_ptr = list_next(itr))) {
		change_grid_color(grid_button_list, i, i, i);
		i++;
	}
	list_iterator_destroy(itr);

	if(view == ERROR_VIEW && display_widget) {
		gtk_widget_destroy(display_widget);
		display_widget = NULL;
	}
	if(!display_widget) {
		tree_view = create_treeview(local_display_data);
		
		display_widget = gtk_widget_ref(GTK_WIDGET(tree_view));
		gtk_table_attach_defaults(GTK_TABLE(table),
					  GTK_WIDGET(tree_view),
					  0, 1, 0, 1);		
		/* since this function sets the model of the tree_view 
		   to the treestore we don't really care about 
		   the return value */
		create_treestore(tree_view, display_data_node, SORTID_CNT);
	}
	view = INFO_VIEW;
	_update_info_node(info_list, GTK_TREE_VIEW(display_widget));
end_it:
	toggled = FALSE;
	force_refresh = 1;
	return;
	
}

extern void specific_info_node(popup_info_t *popup_win)
{
	int error_code = SLURM_SUCCESS;
	static node_info_msg_t *node_info_ptr = NULL;
	specific_info_t *spec_info = popup_win->spec_info;
	char error_char[100];
	GtkWidget *label = NULL;
	GtkTreeView *tree_view = NULL;
	List info_list = NULL;
	List send_info_list = NULL;
	ListIterator itr = NULL;
	int changed = 1;
	sview_node_info_t *sview_node_info_ptr = NULL;
	node_info_t *node_ptr = NULL;
	hostlist_t hostlist = NULL;	
	hostlist_iterator_t host_itr = NULL;
	int i = -1;
		
	if(!spec_info->display_widget)
		setup_popup_info(popup_win, display_data_node, SORTID_CNT);
	
	if(node_info_ptr && popup_win->toggled) {
		gtk_widget_destroy(spec_info->display_widget);
		spec_info->display_widget = NULL;
		goto display_it;
	}
	
	if((error_code = get_new_info_node(&node_info_ptr, 
					   popup_win->force_refresh))
	   == SLURM_NO_CHANGE_IN_DATA) {
		if(!spec_info->display_widget || spec_info->view == ERROR_VIEW)
			goto display_it;
		changed = 0;
		goto display_it;
	}  
			
	if (error_code != SLURM_SUCCESS) {
		if(spec_info->view == ERROR_VIEW)
			goto end_it;
		spec_info->view = ERROR_VIEW;
		if(spec_info->display_widget)
			gtk_widget_destroy(spec_info->display_widget);
		sprintf(error_char, "slurm_load_node: %s",
			slurm_strerror(slurm_get_errno()));
		label = gtk_label_new(error_char);
		gtk_table_attach_defaults(popup_win->table, 
					  label,
					  0, 1, 0, 1); 
		gtk_widget_show(label);	
		spec_info->display_widget = gtk_widget_ref(GTK_WIDGET(label));
		return;
	}
display_it:	
	info_list = create_node_info_list(node_info_ptr, changed);

	if(!info_list)
		return;

	if(spec_info->view == ERROR_VIEW && spec_info->display_widget) {
		gtk_widget_destroy(spec_info->display_widget);
		spec_info->display_widget = NULL;
	}
	if(spec_info->type != INFO_PAGE && !spec_info->display_widget) {
		tree_view = create_treeview(local_display_data);
		
		spec_info->display_widget = 
			gtk_widget_ref(GTK_WIDGET(tree_view));
		gtk_table_attach_defaults(popup_win->table,
					  GTK_WIDGET(tree_view),
					  0, 1, 0, 1);		
		/* since this function sets the model of the tree_view 
		   to the treestore we don't really care about 
		   the return value */
		create_treestore(tree_view, popup_win->display_data, 
				 SORTID_CNT);
	}

#ifdef HAVE_BG
	if(!popup_win->grid_button_list) {
		popup_win->grid_button_list = copy_main_button_list();
		put_buttons_in_table(popup_win->grid_table,
				     popup_win->grid_button_list);
	}
#else
	if(popup_win->grid_button_list) {
		list_destroy(popup_win->grid_button_list);
	}	       
	popup_win->grid_button_list = list_create(destroy_grid_button);
#endif	

	spec_info->view = INFO_VIEW;
	if(spec_info->type == INFO_PAGE) {
		_display_info_node(info_list, popup_win);
		goto end_it;
	}
	
	/* just linking to another list, don't free the inside, just
	   the list */
	send_info_list = list_create(NULL);	

	hostlist = hostlist_create((char *)spec_info->data);
	host_itr = hostlist_iterator_create(hostlist);

	i = -1;
	itr = list_iterator_create(info_list);
	while ((sview_node_info_ptr = list_next(itr))) {
		int found = 0;
		char *host = NULL;
			
		i++;
		node_ptr = sview_node_info_ptr->node_ptr;
		while((host = hostlist_next(host_itr))) { 
			if(!strcmp(host, node_ptr->name)) {
				free(host);
				found = 1;
				break; 
			}
			free(host);
		}
		hostlist_iterator_reset(host_itr);
		if(!found)
			continue;
		
		list_push(send_info_list, sview_node_info_ptr);
#ifdef HAVE_BG
		change_grid_color(popup_win->grid_button_list,
				  i, i, i);
#else
		get_button_list_from_main(&popup_win->grid_button_list,
					  i, i, i);		
#endif
	}
	list_iterator_destroy(itr);

	hostlist_iterator_destroy(host_itr);
	hostlist_destroy(hostlist);

	put_buttons_in_table(popup_win->grid_table,
			     popup_win->grid_button_list);
	
	_update_info_node(send_info_list, 
			  GTK_TREE_VIEW(spec_info->display_widget));
	list_destroy(send_info_list);
end_it:
	popup_win->toggled = 0;
	popup_win->force_refresh = 0;
	
	return;
	
}

extern void set_menus_node(void *arg, GtkTreePath *path, 
			   GtkMenu *menu, int type)
{
	GtkTreeView *tree_view = (GtkTreeView *)arg;
	popup_info_t *popup_win = (popup_info_t *)arg;
	switch(type) {
	case TAB_CLICKED:
		make_fields_menu(menu, display_data_node);
		break;
	case ROW_CLICKED:
		make_options_menu(tree_view, path, menu, options_data_node);
		break;
	case POPUP_CLICKED:
		make_popup_fields_menu(popup_win, menu);
		break;
	default:
		g_error("UNKNOWN type %d given to set_fields\n", type);
	}
}

extern void popup_all_node(GtkTreeModel *model, GtkTreeIter *iter, int id)
{
	char *name = NULL;
	char title[100];
	ListIterator itr = NULL;
	popup_info_t *popup_win = NULL;
	GError *error = NULL;

#ifdef HAVE_BG
	char *node = "base partition";
#else
	char *node = "node";
#endif
	gtk_tree_model_get(model, iter, SORTID_NAME, &name, -1);
	switch(id) {
	case JOB_PAGE:
		snprintf(title, 100, "Jobs(s) with %s %s", node, name);
		break;
	case PART_PAGE:
		snprintf(title, 100, "Partition(s) with %s %s", node, name);
		break;
	case BLOCK_PAGE: 
		snprintf(title, 100, "Blocks(s) with %s %s", node, name);
		break;
	case SUBMIT_PAGE: 
		snprintf(title, 100, "Submit job on %s %s", node, name);
		break;
	case INFO_PAGE: 
		snprintf(title, 100, "Full Info for %s %s", node, name);
		break;
	default:
		g_print("%s got %d\n", node, id);
	}
	
	itr = list_iterator_create(popup_list);
	while((popup_win = list_next(itr))) {
		if(popup_win->spec_info)
			if(!strcmp(popup_win->spec_info->title, title)) {
				break;
			} 
	}
	list_iterator_destroy(itr);
	
	if(!popup_win) {
		if(id == INFO_PAGE)
			popup_win = create_popup_info(id, NODE_PAGE, title);
		else
			popup_win = create_popup_info(NODE_PAGE, id, title);
		popup_win->spec_info->data = name;
		if (!g_thread_create((gpointer)popup_thr, popup_win, 
				     FALSE, &error))
		{
			g_printerr ("Failed to create node popup thread: "
				    "%s\n", 
				    error->message);
			return;
		}
	} else {
		g_free(name);
		gtk_window_present(GTK_WINDOW(popup_win->popup));
	}
}
