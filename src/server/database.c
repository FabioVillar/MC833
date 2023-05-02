#define _GNU_SOURCE  // enables strdup, geline

#include "database.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *columns[COLUMN_COUNT];
} Row;

struct Database {
    int nRows;
    Row *rows[DATABASE_MAX_ROWS];
    pthread_mutex_t mutex;
};

static char *nextColumn(char **row) {
    char *start = *row;
    if (!start) return NULL;

    char *end = strchr(start, '\t');
    if (end) {
        *row = end + 1;
        return strndup(start, end - start);
    } else {
        *row = NULL;
        // Remove o \n
        return strndup(start, strlen(start) - 1);
    }
}

static Row *row_fromString(Database *database, char *string) {
    Row *row = calloc(1, sizeof(Row));
    if (!row) return NULL;
    for (int i = 0; i < COLUMN_COUNT; i++) {
        row->columns[i] = nextColumn(&string);
    }
    return row;
}

static Row *row_new(Database *database) {
    // calloc initializes to 0
    Row *row = calloc(1, sizeof(Row));
    if (!row) return NULL;
    return row;
}

static void row_free(Row *row) {
    for (int i = 0; i < COLUMN_COUNT; i++) {
        if (row->columns[i]) {
            free(row->columns[i]);
        }
    }
    free(row);
}

static const char *row_getColumn(const Row *row, Column column) {
    if (!row) {
        return "NULL";
    }

    const char *value = row->columns[column];
    if (value) {
        return value;
    } else {
        return "NULL";
    }
}

Database *database_new() {
    // calloc initializes to 0
    Database *database = calloc(1, sizeof(Database));
    pthread_mutex_init(&database->mutex, NULL);
    return database;
}

void database_free(Database *database) {
    database_clear(database);
    pthread_mutex_destroy(&database->mutex);
    free(database);
}

int database_countRows(Database *database) {
    pthread_mutex_lock(&database->mutex);

    int result = database->nRows;

    pthread_mutex_unlock(&database->mutex);

    return result;
}

char *database_get(Database *database, int row, Column column) {
    pthread_mutex_lock(&database->mutex);

    char *result = NULL;
    const Row *row2 = database->rows[row];
    if (row2) {
        result = strdup(row_getColumn(row2, column));
    } else {
        result = strdup("NULL");
    }

    pthread_mutex_unlock(&database->mutex);

    return result;
}

void database_load(Database *database, const char *path) {
    database_clear(database);

    pthread_mutex_lock(&database->mutex);

    FILE *file = fopen(path, "r");
    if (file) {
        char *line = NULL;
        size_t allocSize = 0;
        getline(&line, &allocSize, file);  // skip header
        while (getline(&line, &allocSize, file) != -1) {
            if (database->nRows < DATABASE_MAX_ROWS) {
                Row *row = row_fromString(database, line);
                if (!row) continue;
                database->rows[database->nRows++] = row;
            }
        }
        free(line);
        fclose(file);
    } else {
        printf("Load failed\n");
    }

    pthread_mutex_unlock(&database->mutex);
}

void database_save(Database *database, const char *path) {
    pthread_mutex_lock(&database->mutex);

    FILE *file = fopen(path, "w");
    if (file) {
        fprintf(file,
                "Email\tFirst Name\tLast Name\tCity\tGraduation\t"
                "Graduation Year\tSkills\n");
        for (int i = 0; i < database->nRows; i++) {
            const Row *row = database->rows[i];
            for (int j = 0; j < COLUMN_COUNT - 1; j++) {
                fprintf(file, "%s\t", row_getColumn(row, j));
            }
            fprintf(file, "%s\n", row_getColumn(row, COLUMN_COUNT - 1));
        }
        fclose(file);
    } else {
        printf("Save failed\n");
    }

    pthread_mutex_unlock(&database->mutex);
}

void database_clear(Database *database) {
    pthread_mutex_lock(&database->mutex);

    int nRows = database->nRows;
    for (int i = 0; i < nRows; i++) {
        row_free(database->rows[i]);
    }
    database->nRows = 0;

    pthread_mutex_unlock(&database->mutex);
}

void database_addRow(Database *database, const char *email,
                     const char *firstName, const char *lastName,
                     const char *city, const char *graduation,
                     const char *gradYear, const char *skills) {
    pthread_mutex_lock(&database->mutex);

    if (database->nRows < DATABASE_MAX_ROWS) {
        Row *row = row_new(database);
        if (row) {
            row->columns[COLUMN_EMAIL] = strdup(email);
            row->columns[COLUMN_FIRST_NAME] = strdup(firstName);
            row->columns[COLUMN_LAST_NAME] = strdup(lastName);
            row->columns[COLUMN_CITY] = strdup(city);
            row->columns[COLUMN_GRADUATION] = strdup(graduation);
            row->columns[COLUMN_GRAD_YEAR] = strdup(gradYear);
            row->columns[COLUMN_SKILLS] = strdup(skills);
            database->rows[database->nRows++] = row;
        }
    }

    pthread_mutex_unlock(&database->mutex);
}

void database_deleteRow(Database *database, int index) {
    pthread_mutex_lock(&database->mutex);

    row_free(database->rows[index]);
    memmove(&database->rows[index], &database->rows[index + 1],
            sizeof(Row) * (database->nRows - index - 1));
    database->nRows -= 1;
    database->rows[database->nRows] = NULL;

    pthread_mutex_unlock(&database->mutex);
}
