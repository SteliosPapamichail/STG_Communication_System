//
// Created by fresh on 1-5-24.
//

#include <malloc.h>
#include "temp_metrics.h"
#include "event_payloads.h"

metrics_list *create_metrics_list() {
    metrics_list *list = (metrics_list *) malloc(sizeof(metrics_list));
    list->head = (temp_metric *) malloc(sizeof(temp_metric));
    list->tail =  (temp_metric *) malloc(sizeof(temp_metric));
    list->head->next = list->tail;
    list->tail->next = NULL;
    return list;
}

int add_metric(metrics_list *list, st_add_metric *metric) {
    temp_metric *node = (temp_metric *) malloc(sizeof(temp_metric));
    if(node == NULL) return 0;

    node->timestamp = metric->timestamp;
    node->temperature = metric->temperature;

    temp_metric *curr = list->head;
    while(curr->next != list->tail) {
        curr = curr->next;
    }

    if(curr->next == list->tail) { // list is empty
        list->head->next = node;
    } else {
        curr->next = node;
    }

    node->next = list->tail;

    return 1;
}

void destroy_metrics_list(metrics_list *list) {
    temp_metric *curr = list->head->next;

    while(curr != list->tail) {
        temp_metric *temp = curr;
        curr = curr->next;
        free(temp);
    }

    free(list->tail);
    free(list->head);
    free(list);
}