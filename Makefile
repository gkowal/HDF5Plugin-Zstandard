CC     = gcc
CFLAGS = -O2 -fPIC
NAME   = H5Zzstd

lib: lib$(NAME).so

$(NAME).o: $(NAME).c
	$(CC) $(CFLAGS) -c $(NAME).c

lib$(NAME).so: $(NAME).o
	$(CC) -shared -Wl,-soname,lib$(NAME).so $^ -o $@ -lzstd

clean:
	$(RM) *.o *.so*

distclean:
	$(RM) *.o *.so*
