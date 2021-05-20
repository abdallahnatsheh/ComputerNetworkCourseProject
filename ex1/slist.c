#include "slist.h"
#include<stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

void slist_init(slist_t * list){
list->head=NULL;
list->tail=NULL;
list->size=0;
}


void slist_destroy(slist_t * list,slist_destroy_t flag){
if (!list)
		return;
	slist_node_t *temp_h = slist_head(list), *temp_s;
	while (temp_h){
		if (flag) 
			free(slist_data(temp_h));
		temp_s = slist_next(temp_h);
		free(temp_h);
		temp_h = temp_s;
	}
	free(list);
}

void *slist_pop_first(slist_t *list){
	
if(list->head != NULL || list->head!= NULL && list->head->next != NULL){
	void* data =list->head->data;
slist_node_t *tmp=list->head;
list->head=list->head->next;
list->size--;
free(tmp);
return data;
    }else if(list->size==1){
		void* data =list->head->data;
        free(list->head);
		slist_init(list);
		return data;}
    else{return NULL;}
}

int slist_prepend(slist_t *list ,void *data){

slist_node_t *new_node = (slist_node_t*)malloc(1* sizeof(slist_node_t));
	if (!new_node)
		return -1;
	new_node->data= data;
	if (!slist_tail(list)){
		list->tail = new_node;
		list->head= new_node;
		new_node->next = NULL;
	}
	else{
		new_node->next = list->head;
		list->head = new_node;
	}
	list->size++;
	return 0;
}


int slist_append(slist_t * list ,void *data){
slist_node_t *new_node = (slist_node_t*)malloc(1* sizeof(slist_node_t));
	if (!new_node)
		return -1;
	new_node->data = data;
	new_node->next = NULL;
	if (!list->head){
		list->head = new_node;
		list->tail = new_node;
		list->tail->next=NULL;
	}
	else{
		list->tail->next = new_node;
		list->tail = list->tail->next;
		new_node->next=NULL;
	}
	list->size++;
	return 0;
}

int slist_append_list(slist_t *to_list, slist_t *from_list){
if (!from_list)
		return 0;
	slist_node_t *temp = from_list->head;
	while (temp){	
		if (slist_append(to_list, temp->data) < 0)
			return -1;
		temp = temp->next;
	}
	return 0;

}

