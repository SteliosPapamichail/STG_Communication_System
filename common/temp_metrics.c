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
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    return 1;
}

int add_metric_gs(metrics_list *list, const char *timestamp, const double temperature) {
    if (list == NULL) {
        printf("Error: List is NULL\n");
        return 0;
    }

    temp_metric *node = malloc(sizeof(temp_metric));
    if (node == NULL) {
        printf("Error: Failed to allocate memory for node\n");
        return 0;
    }

    strncpy(node->timestamp, timestamp, DATETIME_MAX_LENGTH);
    node->temperature = temperature;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
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

float get_temp_for_timestamp(const metrics_list *list, const char *timestamp) {
    const temp_metric *curr = list->head;

    while (curr != NULL) {
        if (strcmp(curr->timestamp, timestamp) == 0) {
            return curr->temperature; // Return temperature if timestamp matches
        }
        curr = curr->next;
    }

    // Return a special value (e.g., -1) to indicate that the timestamp was not found
    return -1.0f;
}

void destroy_metrics_list(metrics_list *list) {
    temp_metric *curr = list->head;

    while (curr != NULL) {
        temp_metric *temp = curr;
        curr = curr->next;
        free(temp);
    }

    free(list);
}
