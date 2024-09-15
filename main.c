#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>

#define NUM_THREADS 4


typedef struct {
    int  len;
    char** paths;
} ImagesResponse;

typedef struct {
    int len;
    char** paths;
} ImageTheadData;



int image_filter(const struct dirent *entry) {
    // Obtener la extensión del archivo
    const char *ext = strrchr(entry->d_name, '.');

    // Verificar si la extensión corresponde a una imagen
    if (ext && (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".png") == 0 ||
                strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".bmp") == 0)) {
        return 1;  // Incluir este archivo
                }
    return 0;  // Excluir este archivo
}

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
                sprintf(currentWord,"%s\n", entry->d_name);
            break;

            case DT_DIR:
                sprintf(currentWord,"%s/\n", entry->d_name);
            break;

            case DT_LNK:
                sprintf(currentWord,"%s@\n", entry->d_name);
            break;

            default:
                sprintf(currentWord,"%s*\n", entry->d_name);
        }
        // reservamos el espacio de la cadena
        // sobre el puntero de string se reserva la memoria para el string
        response->paths[i] = malloc(strlen(currentWord) + 1);
        strcpy(response->paths[i], currentWord); // se copia el string
        free(entry);
    }

}

/*void chunk_images(const ImagesResponse *images, const int chunkSize,const int numChunks,char ****chunks) {
    int imageIndex = 0;
    for(int j = 0; j < numChunks; j++) {
        for(int i = 0; i < chunkSize; i++) {
            if(imageIndex >= images->len) {
                // si no tenemos mas, liberamos los lotes restantes
                free((*chunks)[j][i]);
                continue;
            }
            strcpy((*chunks)[j][i], images->paths[imageIndex]);
            imageIndex++;
        }
    }
}*/

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

void *image_processing(void * response) {
    ImagesResponse *data = (ImagesResponse*)response;
    for(int i = 0; i < data->len; i++) {
        printf("%s ",data->paths[i]);
    }
}


int main(int argc, char **argv){
    ImagesResponse response;
    get_images("/home/sistemas/Descargas",&response);
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



    /*int index = 0;
    for(int i = 0; i < numChunks; i++) {
        const ImagesResponse *r = chunks[i];

        for(int j = 0; j < r->len; j++) {
            printf("%d %s ",index,r->paths[j]);
            index++;
        }
    }
    */

    // creamos los hilos
    pthread_t threads[numChunks];

    for(int k = 0; k < numChunks; k++) {
        pthread_create(&threads[k], NULL, image_processing, (void *)yachunks[k]);
    }

    // esperamos los threads

    for(int k = 0; k < numChunks; k++) {
        pthread_join(threads[k], NULL);
    }

    return 0;
}
