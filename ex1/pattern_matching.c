#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pattern_matching.h"
pm_match_t* match_init(pm_match_t*, pm_int_t, unsigned char*);//private method for the match
int pm_init(pm_t * tree){//this function initialise the tree

pm_state_t* state =(pm_state_t*) malloc(1* sizeof(pm_state_t));
 tree->zerostate=state;
state->_transitions  = (slist_t*)malloc(1* sizeof(slist_t));
slist_init(state->_transitions);
    state->id = 0;	
	state->depth = 0;
	state->fail = NULL;
	state->output = NULL;


if(!state || !state->_transitions || !tree->zerostate){
    return -1;}else{
		tree->newstate=1;
return 0;
}
}

int pm_goto_set(pm_state_t *from_state,unsigned char symbol,pm_state_t *to_state){

pm_labeled_edge_t *new_edge = (pm_labeled_edge_t*)malloc(1* sizeof(pm_labeled_edge_t));
printf("%d -> %c -> %d\n", from_state->id, symbol, to_state->id);
	
new_edge->state = to_state;
new_edge->label = symbol;
int check=slist_append(from_state->_transitions, new_edge);
if ( check< 0)
	return -1;
return 0;
}
			   


pm_state_t* pm_goto_get(pm_state_t *state,unsigned char symbol){
if (!state){return NULL;}
	slist_node_t *chk_node =state->_transitions->head;
	
while (chk_node){
		if (((pm_labeled_edge_t*)chk_node->data)->label == symbol)
			return ((pm_labeled_edge_t*)chk_node->data)->state;
		chk_node = chk_node->next;
	}
	return NULL;
}

int pm_addstring(pm_t * tree,unsigned char *symbol, size_t n){
if(n>PM_CHARACTERS){
	perror("your string too long , dont try to overflow me ;) ");
	exit(-1);}
  pm_state_t *hlp_state = tree->zerostate;
  pm_state_t *tmp_state =tree->zerostate;
for (int i=0;i<n-1;i++){//loop in the tree states and transition to find the sate
	char letter = symbol[i];
	tmp_state=pm_goto_get(hlp_state,letter);
	if(!tmp_state){//if theres no state have this symbol : make a new state
			printf("Allocating state %d\n", tree->newstate);
			pm_state_t* add_state = (pm_state_t*)malloc(1* sizeof(pm_state_t));
		if (!add_state){perror("error creating a new state in addstring function");
			return -1;}
	add_state->_transitions = (slist_t*)malloc(1* sizeof(slist_t));
	if (!add_state->_transitions){perror("error creating transitions in the new state in addstring function");
		return -1;}

	slist_init(add_state->_transitions);
	add_state->id = tree->newstate++;
	add_state->depth = hlp_state->depth + 1;

		int check=pm_goto_set(hlp_state,letter,add_state);
		if(check<0){perror("error setting up new state in add string function");return-1;}
	
	tmp_state=add_state;
	}
	hlp_state=tmp_state;
  }
hlp_state->output=(slist_t*)malloc(1*sizeof(slist_t));
if(!hlp_state->output){perror("error allocating malloc in addstring fuction");return -1;}
slist_init(hlp_state->output);
slist_prepend(hlp_state->output, symbol);
return 0;	

}



void pm_destroy(pm_t *choosen){

	slist_t *handler = (slist_t*)malloc(1* sizeof(slist_t));
	if (slist_prepend(handler, choosen->zerostate) < 0)
		exit(-1);
	
	while (handler->size > 0){
		pm_state_t* tmp_state = slist_pop_first(handler);
		if (!tmp_state)
			break;
		slist_node_t* tmp_edge = tmp_state->_transitions->head;
		while (tmp_edge){
			if (slist_prepend(handler, ((pm_labeled_edge_t*)tmp_edge->data)->state) < 0){
				perror("error in the tree destroy system");
				exit(-1);
			}
			tmp_edge = tmp_edge->next;
		}
		slist_destroy(tmp_state->_transitions, SLIST_FREE_DATA);
		slist_destroy(tmp_state->output, SLIST_LEAVE_DATA);
		free(tmp_state);
	}
	free(handler);
}

int pm_makeFSM(pm_t *tree){
//the queue to handle the scan at each depth
	slist_t* queue = (slist_t*)malloc(1* sizeof(slist_t));
	if (!queue){
		perror("we fail in the creation of the queue in the makeFSM func!");
		return -1;
	}
	slist_init(queue);
	//tmp_edge - loop over the edges
	slist_node_t* tmp_edge = tree->zerostate->_transitions->head;
	//initializing the failure state 
	while (tmp_edge){
		pm_state_t* hlp_edge = ((pm_labeled_edge_t*)tmp_edge->data)->state;
		if (slist_append(queue, hlp_edge) < 0){ 
			perror("error appending queue with hlp_edge in makeFSM ");
			return -1;
		}
		tmp_edge = tmp_edge->next;
		hlp_edge->fail = tree->zerostate;
	}
	
	
	while (slist_size(queue) > 0){
		pm_state_t *hlp_state = slist_pop_first(queue);
		if (!hlp_state)
			break;
		tmp_edge = hlp_state->_transitions->head;
		//run over its edges
		while (tmp_edge){ 
			
			pm_state_t* fail_state = ((pm_labeled_edge_t*)tmp_edge->data)->state;
			if (slist_append(queue,  fail_state) < 0){
				pm_destroy(tree);
				return -1;
			}
			
			pm_state_t *chk_state = hlp_state->fail;
			unsigned char letter = ((pm_labeled_edge_t*)tmp_edge->data)->label;
			
			
			while (!pm_goto_get(chk_state, letter)) {
				if (!chk_state){ //if reached zerostate (its failure is NULL)
					chk_state = tree->zerostate;
					break;
				}
				chk_state = chk_state->fail;
			}
			
			if (pm_goto_get(chk_state, letter)){ 
				fail_state->fail = pm_goto_get(chk_state, letter);
				printf("Setting f(%d) = %d\n", fail_state->id, fail_state->fail->id);
				
				if (!fail_state->output && fail_state->fail->output){
					fail_state->output = (slist_t*)malloc(1* sizeof(slist_t));
					if (!fail_state->output){
						pm_destroy(tree);
						return -1;
					}
				}
				if (slist_append_list(fail_state->output, fail_state->fail->output) < 0){
					pm_destroy(tree);
					return -1;
				}
			}
			tmp_edge = tmp_edge->next;
		}
	}
	slist_destroy(queue, SLIST_LEAVE_DATA);
	return 0;

}
slist_node_t* fail_state_help(pm_t *newtree,slist_t* handler,slist_node_t* head){
while (head){
	pm_state_t* stater = ((pm_labeled_edge_t*)head->data)->state;
	if (slist_prepend(handler, stater) < 0){
			perror("error appending the handler in fail state help function");
			return -1;
		}
		head = head->next;
		stater->fail = newtree->zerostate;
	}
}


slist_t* pm_fsm_search(pm_state_t *state, unsigned char *pattern, size_t n){
	//match_l the list of the patterns that matched
	slist_t *match_l = (slist_t*)malloc(1* sizeof(slist_t));
	if (!match_l)
		return NULL;
	slist_init(match_l);
	
	
	pm_state_t *z_state = state; 
	for (int i = 0; i < n-1; i++){ 
	
		//while there's no match, keep on looking
		while (!pm_goto_get(state, pattern[i])){
			if (!state) 
				break;
			state = state->fail;
		}
		//match found , or reached to zero state
		state = pm_goto_get(state, pattern[i]); //check if no match
		if (!state){ 

			state = z_state;
			continue;
		}
		if (state->output){ //reached the state we want !
		
			slist_node_t *temp_output = state->output->head;
			while (temp_output){ //go on its output list
			
				//private method to initialize the match
				pm_match_t *new_match = match_init(new_match, i, temp_output->data);
				if (!new_match){
					slist_destroy(match_l, SLIST_FREE_DATA);
					return NULL;
				}
				if (slist_append(match_l, new_match) < 0) {//add the match to the list
				
					slist_destroy(match_l, SLIST_FREE_DATA);
					return NULL;
				}
				printf("Pattern: %s, start at: %d, ends at: %d, last state = %d\n",
				new_match->pattern, new_match->start_pos, new_match->end_pos, state->id);
				temp_output = temp_output->next;
			}
		}
	}
	return match_l;
}
//private method for the match
pm_match_t* match_init(pm_match_t* match, pm_int_t i, unsigned char* pattern)
{
	match = (pm_match_t*)malloc(1* sizeof(pm_match_t));
	if (!match)
		return NULL;
	int length = strlen((char*)pattern);
	match->pattern = (char*)pattern;
	match->end_pos = i;
	match->start_pos = match->end_pos + 1 - length;
	return match;
}