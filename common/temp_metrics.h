//
// Created by fresh on 1-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_TEMP_METRICS_H
#define STG_COMMUNICATION_SYSTEM_TEMP_METRICS_H

#include "event_payloads.h"

typedef struct metric {
    char timestamp[DATETIME_MAX_LENGTH]; // key
    float temperature; // value
    struct metric *next;
} temp_metric;

// simple (naive) list of temperature metrics
// could use a hash-table impl or sorted array using binary search
// since the keys are timestamps, the list is gonna be sorted by default probably
// but obviously a sorted impl would be better
typedef struct {
    temp_metric *head;
    temp_metric *tail;
} metrics_list;

metrics_list *create_metrics_list();

int add_metric(metrics_list *list, st_add_metric metric);

int add_metric_gs(metrics_list *list, const char *timestamp, const double temperature);

float get_temp_for_timestamp(const metrics_list *list, const char *timestamp);

void print_metrics_list(const metrics_list *list);

void destroy_metrics_list(metrics_list *list);

void print_metrics_to_file(FILE *out, const metrics_list *list);
#endif //STG_COMMUNICATION_SYSTEM_TEMP_METRICS_H
