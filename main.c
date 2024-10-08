#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <wand/MagickWand.h>
#include <libgen.h>
#include <unistd.h>

#define NUM_THREADS 4

typedef struct {
    int  len;
    char** paths;
    char * dest;
} ImagesResponse;


/**
 * Filtra solo aquellas que son imagenes
 * @param entry entrada del scan
 * @return devuelve codigo de error
 */
int image_filter(const struct dirent *entry) {
    // Obtener la extensión del archivo
    const char *ext = strrchr(entry->d_name, '.');

    // Verificar si la extensión corresponde a una imagen
    if (ext && (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".png") == 0 ||
                strcmp(ext, ".jpeg") == 0)) {
        return 1;  // Incluir este archivo
                }
    return 0;  // Excluir este archivo
}

/**
 *
 * @param path donde se encuentran las images
 * @param response estructura que contiene los paths y su longitud
 */
void get_images(const char *path, ImagesResponse *response) {
    int i = 0;
    struct dirent **files;


    response->len = scandir(path,&files,image_filter,alphasort);


    if(response->len < 0){
        perror("no fue posible leer el directorio");
        return;
    }

    // se instancia el arreglo para guardar punteros de strings
    response->paths = malloc((response->len+1) * sizeof(char*));


    for(i = 0; i < response->len; i++){
        char currentWord[500];
        struct dirent *entry = files[i];
        switch (entry->d_type) {
            case DT_REG:
                sprintf(currentWord,"%s", entry->d_name);
            break;

            case DT_DIR:
                sprintf(currentWord,"%s/", entry->d_name);
            break;

            case DT_LNK:
                sprintf(currentWord,"%s@", entry->d_name);
            break;

            default:
                sprintf(currentWord,"%s*", entry->d_name);
        }
        // reservamos el espacio de la cadena
        // sobre el puntero de string se reserva la memoria para el string
        response->paths[i] = malloc((strlen(currentWord) + strlen(path)) * sizeof(char));
        sprintf(response->paths[i],"%s/%s", path,currentWord);
        // nos aseguramos que string tenga bien colado el caracter nulo para su longitud
        /*int indexJump = strcspn(response->paths[i],"\n");
        if (indexJump != strlen(response->paths[i])) {
            response->paths[i][indexJump] = '\0';
        }*/
        free(entry);
    }

}


/**
 * Separa la ImagesResponse de todas las imagenes en chunks
 * @param response response de todas las imagenes
 * @param chunk_size longitud de cada chunk
 * @param chunk_num numero de chunks
 * @param chunks arreglo de response donde se va guardar las separaciones
 */
void chunk_images(const ImagesResponse *response,const int chunk_size,const int chunk_num,ImagesResponse ***chunks) {
    int index = 0;
    for(int i = 0; i < chunk_num; i++) {
        // cada response lo instancia en memoria
        (*chunks)[i] = malloc(sizeof(ImagesResponse));
        // instanciamos el arreglo de paths
        (*chunks)[i]->paths = malloc(chunk_size * sizeof(char*));
        (*chunks)[i]->len = 0;
        for(int j = 0; j < chunk_size; j++) {
            if(index >= response->len) {
                // si ya no tenemos paths por copiar, limpiamos los lotes restantes
                (*chunks)[i]->paths[j] = NULL;
                continue;
            }
            // instancamos donde se va guardar la cadena
            (*chunks)[i]->paths[j] = malloc(256 * sizeof(char));
            strcpy((*chunks)[i]->paths[j],response->paths[index]);
            (*chunks)[i]->len = (*chunks)[i]->len + 1;
            index++;
        }
    }
}



/**
 * rutina que comprime las imagenes en paralelo
 * @param response Chunk response de las imagenes a procesar en paralelo
 * @return
 */
void *image_processing(void * response) {
    const ImagesResponse *data = (ImagesResponse*)response;
    for(int i = 0; i < data->len; i++) {
        char * path_copy = strdup(data->paths[i]);
        char *base = basename(path_copy);
        char * fullpath = malloc((strlen(data->dest) + strlen(base) + 1) * sizeof(char));
        sprintf(fullpath,"%s/%s",data->dest,base);

        MagickWand *wand = NewMagickWand();

        if(access(data->paths[i],F_OK) != 0) {
            printf("No found image: %s\n",data->paths[i]);
            return NULL;
        }

        printf("*\timage: %s\n",data->paths[i]);

        if(MagickReadImage(wand,data->paths[i]) == MagickFalse) {
            perror("MagickReadImage error");
            return NULL;
        }

        size_t originalHeight = MagickGetImageHeight(wand);
        size_t originalWith = MagickGetImageWidth(wand);
        size_t width = 150;

        double aspectRatio = (double) originalHeight / (double) originalWith;

        size_t newHeight = (size_t)(width * aspectRatio +0.5);


        if(MagickResizeImage(wand,width,newHeight,LanczosFilter,1) == MagickFalse) {
            printf("Error resizing image\n");
            return NULL;
        }

        if(MagickSetCompressionQuality(wand,100) == MagickFalse) {
            perror("MagickSetCompressionQuality error");
            return NULL;
        }


        if(MagickWriteImage(wand,fullpath) == MagickFalse) {
            perror("MagickWriteImage error");
        }


        DestroyMagickWand(wand);

    }

}



int main(int argc, char **argv){

    const char * pathOrigen = "/home/sistemas/Imágenes";
    const char * pathDest = "/home/sistemas/Documentos/Comprimidos";

    MagickWandGenesis();

    ImagesResponse response;
    get_images(pathOrigen,&response);
    // creamos un arreglo bidemesionasl para los chunks y dividirlos en hilos
    const int chunkSize = ceil((double) (response.len) / (double) (NUM_THREADS));
    const int numChunks = ceil((double) response.len / (double) chunkSize);

    printf("* num threads:\t %d\n",NUM_THREADS);
    printf("* num chunks:\t %d\n",numChunks);
    printf("* images len:\t %d\n",response.len);
    printf("* chunkSize:\t %d\n",chunkSize);

    // creamos el arreglo de responses
    ImagesResponse **chunks = malloc(numChunks * sizeof(ImagesResponse*));
    chunk_images(&response,chunkSize,numChunks,&chunks);

    // creamos los hilos
    pthread_t threads[numChunks];

    for(int k = 0; k < numChunks; k++) {
        chunks[k]->dest = malloc(strlen(pathDest) * sizeof(char));
        strcpy(chunks[k]->dest,pathDest);
        pthread_create(&threads[k], NULL, image_processing, (void *)chunks[k]);
    }

    // esperamos los threads

    for(int k = 0; k < numChunks; k++) {
        pthread_join(threads[k], NULL);
    }

    return 0;
}
