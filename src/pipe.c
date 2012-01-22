#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "pipe.h"

struct childoutput *pipecmd(char *args[], char **envp, struct childinput *input)
{
	struct childoutput *output;
	int p2c_stdin[2], c2p_stdout[2];
	pid_t pid;
	size_t n, tnb, bufsz;
	
	if (pipe(p2c_stdin) < 0)
		pexit(pipe);
	if (pipe(c2p_stdout) < 0)
		pexit(pipe);
	
	switch ((pid = fork()))
	{
		case -1:
			pexit(fork);
		case 0:
		{
			close(p2c_stdin[1]); /* close write end */
			
			dup2(p2c_stdin[0], STDIN_FILENO);
			if (input->std_out)
				dup2(c2p_stdout[0], STDOUT_FILENO);
			else
				dup2(c2p_stdout[1], STDOUT_FILENO);
			
			execve(args[0], args, envp);
			
			dup2(STDIN_FILENO, STDIN_FILENO);
			dup2(STDOUT_FILENO, STDOUT_FILENO);
			
			close(p2c_stdin[0]);
			close(c2p_stdout[1]);
			
			pcexit(execve);
			
			break;
		}
		default:
		{
			output = malloc(sizeof(struct childoutput));
			if (!output)
				pexit(malloc);
				
			close(p2c_stdin[0]); /* close read end */
			if (input->std_out)
				close(c2p_stdout[0]);
			else
				close(c2p_stdout[1]); /* close write end */
			
			if (write(p2c_stdin[1], input->std_in, input->std_in_tnb) != input->std_in_tnb)
				pexit(write);
			close(p2c_stdin[1]);
			
			if (input->std_out)
			{
				if (write(c2p_stdout[1], input->std_out, input->std_out_tnb) != input->std_out_tnb)
					pexit(write);
				close(c2p_stdout[1]);
				
				output->std_out = NULL;
			}
			else
			{
				output->std_out = malloc(INITIALSZ);
				if (!(output->std_out))
					pexit(malloc);
				
				memset(output->std_out, 0x00, INITIALSZ);
				tnb = 0;
				bufsz = INITIALSZ;
				
				while ((n = read(c2p_stdout[0], output->std_out+tnb, bufsz-tnb)) != 0)
				{
					if (n < 0)
						pexit(read);
					
					tnb += n;
					
					if (tnb <= bufsz-REALLOCSZ-1)
					{
						output->std_out = realloc(output->std_out, bufsz+REALLOCSZ);
						if (!(output->std_out))
							pexit(realloc);
						
						memset((output->std_out)+bufsz, 0x00, REALLOCSZ);
						bufsz += REALLOCSZ;
					}
				}
			}
			
			if (waitpid(pid, &(output->errcode), 0) < 0)
				pexit(waitpid);
			
			break;
		}
	}
	
	return output;
}

void free_childoutput(struct childoutput *output)
{
	if (!output)
		return;
	if (output->std_in)
	{
		free(output->std_in);
		output->std_in = NULL;
	}
	if (output->std_out)
	{
		free(output->std_out);
		output->std_out = NULL;
	}
	if (output->std_err)
	{
		free(output->std_err);
		output->std_err = NULL;
	}
	free(output);
}

void free_childinput(struct childinput *input)
{
	if (!input)
		return;
	if (input->std_in)
	{
		free(input->std_in);
		input->std_in = NULL;
	}
	if (input->std_out)
	{
		free(input->std_out);
		input->std_out = NULL;
	}
	free(input);
}
