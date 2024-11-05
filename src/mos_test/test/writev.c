writev(fd, iovec, iovlen)
struct iovec iovec[];
{
	int i;
	int bwr = 0;

	for(i = 0; i < iovlen; ++i)
	{
		register int ret;
		
		if(iovec[i].iov_len <= 0)
			continue;
		if((ret = write(fd, (char *)(iovec[i].iov_base), 
					(unsigned)(iovec[i].iov_len))) == -1)
			return(bwr?bwr:-1);
		else if(ret == 0)
			return(bwr);
		else if(ret != iovec[i].iov_len)
			return(bwr+ret);
		bwr += ret;
	}
	return(bwr);
}
		

# endif /* u3b2 */

# endif /* lint */
