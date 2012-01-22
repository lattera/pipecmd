#ifndef _PIPE_H
#define _PIPE_H

struct childoutput
{
	int errcode;
	
	char *std_in;
	char *std_out;
	char *std_err;
};

struct childinput
{
	char *std_in;
	char *std_out;
	
	size_t std_in_tnb;
	size_t std_out_tnb;
};

/* BUFFER SIZES */
#define INITIALSZ	1024
#define REALLOCSZ	256

#define pexit(x) \
{ \
	perror(#x); \
	exit(1); \
}

#define pcexit(x) \
{ \
	perror(#x); \
	_exit(1); \
}

struct childoutput *pipecmd(char **, char **, struct childinput *);
void free_childoutput(struct childoutput *);
void free_childinput(struct childinput *);

#endif /* _PIPE_H */
