FLAGS = -Wall -std=gnu99 -g

all: copy greyscale gaussian_blur edge_detection scale image_filter

copy: copy.o bitmap.o
	gcc ${FLAGS} -o $@ $^ -lm

greyscale: greyscale.o bitmap.o
	gcc ${FLAGS} -o $@ $^ -lm

gaussian_blur: gaussian_blur.o bitmap.o
	gcc ${FLAGS} -o $@ $^ -lm

edge_detection: edge_detection.o bitmap.o
	gcc ${FLAGS} -o $@ $^ -lm

scale: scale.o bitmap.o
	gcc ${FLAGS} -o $@ $^ -lm

image_filter: image_filter.o
	gcc ${FLAGS} -o $@ $^ -lm

%.o: %.c bitmap.h
	gcc ${FLAGS} -c $<

clean:
	rm *.o image_filter copy greyscale gaussian_blur edge_detection scale

test:
	mkdir -p images
	./copy < dog.bmp > images/dog_copy.bmp
	./greyscale < dog.bmp > images/dog_greyscale.bmp
	./gaussian_blur < dog.bmp > images/dog_gaussian_blur.bmp
	./edge_detection < dog.bmp > images/dog_edge_detection.bmp
	./scale < dog.bmp > images/dog_scale_filter.bmp
	./gaussian_blur < dog.bmp | ./gaussian_blur | ./gaussian_blur | ./greyscale | ./scale 2 > images/dog_piped-1.bmp
	./image_filter dog.bmp images/dog_piped-2.bmp ./gaussian_blur ./gaussian_blur ./gaussian_blur ./greyscale "./scale 2"
	./gaussian_blur < dog.bmp | ./gaussian_blur | ./gaussian_blur | ./scale 2 | ./greyscale | ./scale 2 | ./gaussian_blur > images/dog_piped-3.bmp
	./image_filter dog.bmp images/dog_piped-4.bmp ./gaussian_blur ./gaussian_blur ./gaussian_blur "./scale 2" ./greyscale "./scale 2" ./gaussian_blur
	./gaussian_blur < dog.bmp | ./gaussian_blur | ./gaussian_blur | ./gaussian_blur | ./gaussian_blur | ./gaussian_blur | ./gaussian_blur | ./greyscale | ./scale 2 > images/dog_piped-5.bmp
	./image_filter dog.bmp images/dog_piped-6.bmp ./gaussian_blur ./gaussian_blur ./gaussian_blur ./gaussian_blur ./gaussian_blur ./gaussian_blur ./gaussian_blur ./greyscale "./scale 2"