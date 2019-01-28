IMG:=../../fat.img

boot.bin : boot.asm pm.inc
	nasm $< -o $@ -l boot.list
	dd if=$@ of=$(IMG) conv=notrunc

.PHONY : clean
clean :
	rm -f boot.bin 
