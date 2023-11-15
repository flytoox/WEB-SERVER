

//? rc.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
//* You can use the MACRO INADDR_LOOPBACK for LocalHost in the set of htonl()
//* Since certain plateforms are little-endian, hence htonl() ends up reversing bits



//? void FD_ZERO(fd_set *fdset);         //* clear all bits in fdset */
//? void FD_SET(int fd, fd_set *fdset);  //* turn on the bit for fd in fdset */
//? void FD_CLR(int fd, fd_set *fdset);  //* turn off the bit for fd in fdset */
//? int FD_ISSET(int fd, fd_set *fdset); //* is the bit for fd on in fdset ? */

