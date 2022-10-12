#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


/*
 p[0] read    sort
 p[1] write   who

who writes to pipe and then we sort

who  > infile
sort < infile

who | sort
*/

int main()
  {
  int fds[2];
  pipe( fds ) ;   /* no error checks */

  if( fork() == 0 ) // fork once

        {  // SORT
	/* 1st child */
	// sort child - of who | sort

        /* fds[0]/stdin --> sort */
	/* read 'stdin' from pipe */
	/* stdout is as usual  */
        dup2( fds[0] , STDIN_FILENO );
        close( fds[ 1 ] );
        execlp( "sort", "sort", (char *) 0 );
        }
  else
        {
	/* parent */

	if( fork() == 0 ) // fork twice
        {  // WHO
	/* 2nd child */

        /* who --> fds[1]/stdout --> sort */
	/* write 'stdout' to pipe */
	/* stdout  to pipe */
        dup2( fds[1] , STDOUT_FILENO );
        close( fds[ 0 ] );
        execlp( "who", "who", (char *) 0 );
        }
   else
        { /* parent closes all links */
	    	if( fork() == 0 ) // fork twice
		{  // WHO
		    /* 2nd child */

		    /* who --> fds[1]/stdout --> sort */
		    /* write 'stdout' to pipe */
		    /* stdout  to pipe */
		    dup2( fds[1] , STDOUT_FILENO );
		    close( fds[ 0 ] );
		    execlp( "who", "who", (char *) 0 );
		}

	    close( fds[ 0 ] );
	    close( fds[ 1 ] );

        wait( (int *) 0 );
        wait( (int *) 0 );
        } /* else parent second child */
    } /* else parent first child */
  return 0;
  }
