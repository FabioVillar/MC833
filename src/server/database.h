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

typedef enum {
    DB_OK,
    DB_ALREADY_EXISTS,
    DB_FULL,
    DB_EMAIL_DOES_NOT_EXIST,
    DB_IMAGE_DOES_NOT_EXIST,
} DatabaseResult;

typedef struct Database Database;

/// Cria um objeto database
Database *database_new(const char *directory);

/// Deleta um database
void database_free(Database *database);

/// Retorna a contagem de linhas
int database_countRows(Database *database);

/// Retorna o valor em linha e coluna
/// Deve ser liberado com free
char *database_get(Database *database, int row, Column column);

/// Carrega a partir do arquivo
void database_load(Database *database);

/// Salva o banco de dados no arquivo
void database_save(Database *database);

/// Limpa o banco de dados
void database_clear(Database *database);

/// Adiciona uma linha
///
/// Falha caso o e-mail já exista no banco de dados ou o banco de dados
/// esteja cheio.
DatabaseResult database_addRow(Database *database, const char *email,
                               const char *firstName, const char *lastName,
                               const char *city, const char *graduation,
                               const char *gradYear, const char *skills);

/// Deleta uma linha com certo e-mail.
DatabaseResult database_deleteRow(Database *database, const char *email);

/// Adiciona/modifica uma imagem de um perfil.
DatabaseResult database_setImage(Database *database, const char *email,
                                 const void *data, int size);

/// Obtém uma imagem de um perfil.
DatabaseResult database_getImage(Database *database, const char *email,
                                 void *data, int *size);

#endif
