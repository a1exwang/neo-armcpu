snake: snake.c *.h ../*.h
	pwd
	CFLAGS="-I.. -I../../user/libs -I../../kern/include" 
	# LDFLAGS="-L../../obj/user/ -luser"
	../../compile_userapp_raw.sh $< $@

clean:
	rm -f snake

.PHONY: clean
