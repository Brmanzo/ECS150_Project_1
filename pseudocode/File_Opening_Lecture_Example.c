int main(int argc, char *argv[])
{
   int fd;
  char buf[4];
  
  if (argc < 2)
    exit(EXIT_FAILURE);
  
  // Clearing Buffer
  memset(buf, 0, sizeof(buf));
  
  // Opening file specified at second argument
  fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  // copying first four letters into the buffer
  read(fd, buf, sizeof(buf));
  close(fd);
  
  return 0;
}
