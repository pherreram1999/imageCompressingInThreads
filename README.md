# imageCompressingInThreads

El script depende de imagemagick y su dependencias de desarrollo

 Para sistemas debian y derivados como ubuntu, instalar
~~~shell
sudo apt install imagemagick libmagickwand-dev
~~~
en caso de no usar cmake para compilar es posible mediante comando usando 
~~~shell
gcc -o demo main.c -lm $(pkg-config --cflags --libs MagickWand)
~~~
_nota_: se da por hecho que la distrubuccion linux tiene instalado pkg-config