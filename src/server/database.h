#ifndef DATABASE_H
#define DATABASE_H

#define DATABASE_MAX_ROWS 256
#define DATABASE_FILE "profiles.tsv"

typedef enum {
    COLUMN_EMAIL,
    COLUMN_FIRST_NAME,
    COLUMN_LAST_NAME,
    COLUMN_CITY,
    COLUMN_GRADUATION,
    COLUMN_GRAD_YEAR,
    COLUMN_SKILLS,

    COLUMN_COUNT,
} Column;

typedef struct Database Database;

Database *database_new();

void database_free(Database *database);

int database_countRows(Database *database);

char *database_get(Database *database, int row, Column column);

void database_load(Database *database, const char *path);

void database_save(Database *database, const char *path);

void database_clear(Database *database);

void database_addRow(Database *database, const char *email,
                     const char *firstName, const char *lastName,
                     const char *city, const char *graduation,
                     const char *gradYear, const char *skills);

void database_deleteRow(Database *database, int index);

#endif
