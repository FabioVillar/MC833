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

/// Cria um objeto database
Database *database_new();

/// Deleta um database
void database_free(Database *database);

/// Retorna a contagem de linhas
int database_countRows(Database *database);

/// Retorna o valor em linha e coluna
/// Deve ser liberado com free
char *database_get(Database *database, int row, Column column);

/// Carrega a partir de um arquivo
void database_load(Database *database, const char *path);

/// Salva o banco de dados em um arquivo
void database_save(Database *database, const char *path);

/// Limpa o banco de dados
void database_clear(Database *database);

/// Adiciona uma linha
void database_addRow(Database *database, const char *email,
                     const char *firstName, const char *lastName,
                     const char *city, const char *graduation,
                     const char *gradYear, const char *skills);

/// Deleta uma linha
void database_deleteRow(Database *database, int index);

#endif
