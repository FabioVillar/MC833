#ifndef SERVER_H
#define SERVER_H

#include "server_socket.h"

typedef struct Server Server;

Server *server_new(int port, const char *directory);

void server_free(Server *server);

void server_run(Server *server);

/// Insert a profile.
void server_insertProfile(Server *server, const Request *request);

/// List by course.
void server_listByCourse(Server *server, const Request *request);

/// List by skill.
void server_listBySkill(Server *server, const Request *request);

/// List by graduation year.
void server_listByYear(Server *server, const Request *request);

/// List all.
void server_listAll(Server *server, const Request *request);

/// List by email.
void server_listByEmail(Server *server, const Request *request);

/// Remove by email.
void server_removeByEmail(Server *server, const Request *request);

/// Upload image.
void server_uploadImage(Server *server, const Request *request);

/// Download image.
void server_downloadImage(Server *server, const Request *request);

/// Clear the buffer.
void server_clearBuffer(Server *server);

/// Add a row as profile to the buffer.
void server_addProfileToBuffer(Server *server, int row);

/// Add name and email to the buffer.
void server_addNameAndEmailToBuffer(Server *server, int row);

/// Add graduation field, name and email to the buffer.
void server_addCourseNameAndEmailToBuffer(Server *server, int row);

void server_addEndOfListToBuffer(Server *server);

#endif
