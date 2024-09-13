#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <string.h>

#define NUM_THREADS 4


typedef struct {
    int  len;
    char** paths;
} ImagesResponse;



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

void chunk_images(ImagesResponse *images,int chunkSize,char ****chunks) {
    int imageIndex = 0;
    for(int j = 0; j < NUM_THREADS; j++) {
        for(int i = 0; i < chunkSize; i++) {
            if(imageIndex > images->len) return;
            strcpy((*chunks)[j][i], images->paths[imageIndex]);
            imageIndex++;
        }
    }

}


int main(int argc, char **argv){
    ImagesResponse response;
    get_images("/home/sistemas/Imágenes",&response);
    // creamos un arreglo bidemesionasl para los chunks y dividirlos en hilos
    int chunkSize = ceil((double) (response.len) / (double) (NUM_THREADS));

    printf("NUM THREADS %d\n",NUM_THREADS);
    printf("images len %d\n",response.len);
    printf("chunkSize %d\n",chunkSize);

    char ***chunks = malloc(NUM_THREADS * sizeof(char*));

    for(int k = 0; k < NUM_THREADS; k++) {
        chunks[k] = malloc(chunkSize * sizeof(char*));
        for(int i = 0; i < chunkSize; i++) {
            chunks[k][i] = malloc(256 * sizeof(char));
        }
    }
    chunk_images(&response,chunkSize,&chunks);

    for(int k = 0; k < NUM_THREADS; k++) {

        chunks[k] = malloc(chunkSize * sizeof(char*));

        for(int i = 0; i < chunkSize; i++) {
            printf("string %s\n", chunks[k][i]);
        }
    }


    return 0;
}
