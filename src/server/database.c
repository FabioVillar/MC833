#define _GNU_SOURCE  // enables strdup

#include "database.h"

#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>

typedef struct {
    char *columns[COLUMN_COUNT];
} Row;

struct Database {
    int nRows;
    char *path;
    char *directory;
    Row *rows[DATABASE_MAX_ROWS];
    // Protege a base de dados contra acesso em multiplas threads
    pthread_mutex_t mutex;
};

/// Parse a tab separated string, advancing the string and returning the column
/// Must call free later on return value
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

static char *getImagePath(Database *database, const char *email) {
    int imagePathSize = strlen(database->directory) + 1 + strlen(email) + 4;
    char *imagePath = malloc(imagePathSize + 1);
    if (!imagePath) exit(-1);
    snprintf(imagePath, imagePathSize + 1, "%s/%s.jpg", database->directory,
             email);
    return imagePath;
}

/// Parse a row from a tab separated line
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
    if (!row) exit(-1);
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

Database *database_new(const char *directory) {
    if (mkdir(directory, 0775) < 0 && errno != EEXIST) {
        printf("%s\n", strerror(errno));
    }

    int directorySize = strlen(directory);
    char *directory2 = malloc(directorySize + 2);
    if (!directory2) exit(-1);
    strcpy(directory2, directory);
    directory2[directorySize] = '/';
    directory2[directorySize + 1] = '\0';

    char *path = malloc(directorySize + 1 + sizeof(DATABASE_FILE) + 1);
    if (!path) exit(-1);
    strcpy(path, directory2);
    strcpy(&path[directorySize + 1], DATABASE_FILE);

    // calloc initializes to 0
    Database *database = calloc(1, sizeof(Database));
    if (!database) exit(-1);
    database->directory = directory2;
    database->path = path;

    pthread_mutex_init(&database->mutex, NULL);

    return database;
}

void database_free(Database *database) {
    database_clear(database);
    pthread_mutex_destroy(&database->mutex);
    free(database->path);
    free(database->directory);
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

void database_load(Database *database) {
    database_clear(database);

    pthread_mutex_lock(&database->mutex);

    FILE *file = fopen(database->path, "r");
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
        // printf("Load failed\n");
    }

    pthread_mutex_unlock(&database->mutex);
}

void database_save(Database *database) {
    pthread_mutex_lock(&database->mutex);

    FILE *file = fopen(database->path, "w");
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

DatabaseResult database_addRow(Database *database, const char *email,
                               const char *firstName, const char *lastName,
                               const char *city, const char *graduation,
                               const char *gradYear, const char *skills) {
    DatabaseResult r = DB_OK;

    pthread_mutex_lock(&database->mutex);

    int nRows = database->nRows;
    for (int i = 0; i < nRows; i++) {
        if (strcmp(database->rows[i]->columns[COLUMN_EMAIL], email) == 0) {
            r = DB_ALREADY_EXISTS;
            break;
        }
    }

    if (r == DB_OK && nRows >= DATABASE_MAX_ROWS) {
        r = DB_FULL;
    }

    if (r == DB_OK) {
        Row *row = row_new(database);
        row->columns[COLUMN_EMAIL] = strdup(email);
        row->columns[COLUMN_FIRST_NAME] = strdup(firstName);
        row->columns[COLUMN_LAST_NAME] = strdup(lastName);
        row->columns[COLUMN_CITY] = strdup(city);
        row->columns[COLUMN_GRADUATION] = strdup(graduation);
        row->columns[COLUMN_GRAD_YEAR] = strdup(gradYear);
        row->columns[COLUMN_SKILLS] = strdup(skills);
        database->rows[nRows] = row;

        database->nRows++;
    }

    pthread_mutex_unlock(&database->mutex);

    return r;
}

DatabaseResult database_deleteRow(Database *database, const char *email) {
    DatabaseResult r = DB_EMAIL_DOES_NOT_EXIST;

    pthread_mutex_lock(&database->mutex);

    int nRows = database->nRows;
    for (int i = 0; i < nRows; i++) {
        if (strcmp(database->rows[i]->columns[COLUMN_EMAIL], email) == 0) {
            row_free(database->rows[i]);
            memmove(&database->rows[i], &database->rows[i + 1],
                    sizeof(Row) * (database->nRows - i - 1));
            database->nRows--;
            database->rows[database->nRows] = NULL;

            r = DB_OK;

            break;
        }
    }

    pthread_mutex_unlock(&database->mutex);

    return r;
}

DatabaseResult database_setImage(Database *database, const char *email,
                                 const void *data, int size) {
    DatabaseResult r = DB_OK;

    pthread_mutex_lock(&database->mutex);

    r = DB_EMAIL_DOES_NOT_EXIST;
    int nRows = database->nRows;
    for (int i = 0; i < nRows; i++) {
        if (strcmp(database->rows[i]->columns[COLUMN_EMAIL], email) == 0) {
            r = DB_OK;
            break;
        }
    }

    if (r == DB_OK) {
        char *imagePath = getImagePath(database, email);
        FILE *f = fopen(imagePath, "wb");
        free(imagePath);

        if (f) {
            fwrite(data, size, 1, f);
            fclose(f);
        }
    }

    pthread_mutex_unlock(&database->mutex);

    return r;
}

DatabaseResult database_getImage(Database *database, const char *email,
                                 void *data, int *size) {
    DatabaseResult r = DB_OK;

    pthread_mutex_lock(&database->mutex);

    r = DB_EMAIL_DOES_NOT_EXIST;
    int nRows = database->nRows;
    for (int i = 0; i < nRows; i++) {
        if (strcmp(database->rows[i]->columns[COLUMN_EMAIL], email) == 0) {
            r = DB_OK;
            break;
        }
    }

    if (r == DB_OK) {
        char *imagePath = getImagePath(database, email);
        FILE *f = fopen(imagePath, "rb");
        free(imagePath);

        if (f) {
            *size = fread(data, *size, 1, f);
            fclose(f);
        } else {
            *size = 0;
            r = DB_IMAGE_DOES_NOT_EXIST;
        }
    }

    pthread_mutex_unlock(&database->mutex);

    return r;
}
