//
// Created by fresh on 1-5-24.
//

#include <malloc.h>
#include "temp_metrics.h"

#include <string.h>

#include "event_payloads.h"

metrics_list *create_metrics_list() {
    metrics_list *list = malloc(sizeof(metrics_list));
    if (list == NULL) {
        return NULL; // Failed to allocate memory for the list
    }
    list->head = NULL; // Initialize head to NULL
    list->tail = NULL; // Initialize tail to NULL
    return list;
}

int add_metric(metrics_list *list, const st_add_metric metric) {
    if (list == NULL) {
        printf("Error: List is NULL\n");
        return 0;
    }

    temp_metric *node = malloc(sizeof(temp_metric));
    if (node == NULL) {
        printf("Error: Failed to allocate memory for node\n");
        return 0;
    }

    strcpy(node->timestamp, metric.timestamp);
    node->temperature = metric.temperature;
    node->next = NULL;

    if (list->head == NULL) {
        printf("List was empty. Adding node as head\n");
        list->head = node;
        list->tail = node;
    } else {
        printf("List was not empty. Appending node to the end\n");
        list->tail->next = node;
        list->tail = node;
    }

    return 1;
}

void print_metrics_list(const metrics_list *list) {
    const temp_metric *curr = list->head;

    printf("-=-=- Metrics list -=-=-\n");
    while (curr != NULL) {
        printf("%.1f %s\n", curr->temperature, curr->timestamp);
        curr = curr->next;
    }
}

void destroy_metrics_list(metrics_list *list) {
    temp_metric *curr = list->head;

    while (curr != NULL) {
        temp_metric *temp = curr;
        curr = curr->next;
        // Free memory for the dynamically allocated string
        free(temp->timestamp);
        free(temp);
    }

    free(list);
}