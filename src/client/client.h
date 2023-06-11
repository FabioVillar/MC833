#ifndef CLIENT_H
#define CLIENT_H

typedef struct Client Client;

Client *client_new(const char *ip, int port);

void client_free(Client *client);

void client_run(Client *client);

/// Insert a profile.
void client_insertProfile(Client *client);

/// List by course.
void client_listByCourse(Client *client);

/// List by course.
void client_listBySkill(Client *client);

/// List by skill.
void client_listByYear(Client *client);

/// List all.
void client_listAll(Client *client);

/// List by email.
void client_listByEmail(Client *client);

/// Remove by email.
void client_removeByEmail(Client *client);

#endif
