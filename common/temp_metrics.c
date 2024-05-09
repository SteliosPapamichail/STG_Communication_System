//
// Created by fresh on 1-5-24.
//

#include <malloc.h>
#include "temp_metrics.h"
#include "event_payloads.h"

metrics_list *create_metrics_list() {
    metrics_list *list = malloc(sizeof(metrics_list));
    list->head = (temp_metric *) malloc(sizeof(temp_metric));
    list->tail = (temp_metric *) malloc(sizeof(temp_metric));
    list->head->next = list->tail;
    list->tail->next = NULL;
    return list;
}

int add_metric(const metrics_list *list, const st_add_metric metric) {
    temp_metric *node = malloc(sizeof(temp_metric));
    if (node == NULL) return 0;

    node->timestamp = metric.timestamp;
    node->temperature = metric.temperature;

    temp_metric *curr = list->head;

    printf("add metric list called\n");

    if (curr->next == list->tail) {
        printf("metrics list was empty\n");
        // list is empty
        list->head->next = node;
        node->next = list->tail;
        return 1;
    }

    printf("metrics list NOT EMPTY\n");

    while (curr->next != list->tail) {
        curr = curr->next;
    }

    curr->next = node;
    node->next = list->tail;

    return 1;
}

void print_metrics_list(const metrics_list *list) {
    temp_metric *curr = list->head->next;

    printf("-=-=- Metrics list -=-=-\n");
    while (curr != list->tail) {
        printf("%.1f %s\n", curr->temperature, curr->timestamp);
        curr = curr->next;
    }
}

void destroy_metrics_list(metrics_list *list) {
    temp_metric *curr = list->head->next;

    while (curr != list->tail) {
        temp_metric *temp = curr;
        curr = curr->next;
        // free memory for the dynamically allocated string
        free(temp->timestamp);
        free(temp);
    }

    free(list->tail);
    free(list->head);
    free(list);
}
